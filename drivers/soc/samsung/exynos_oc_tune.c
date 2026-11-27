// SPDX-License-Identifier: GPL-2.0
/*
 * Built-in safe SRAM overclocking, -2% undervolt, and +5C thermal headroom
 * tuner for Exynos 1380 (s5e8835) when vendor modules load from vendor_boot.
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/thermal.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <soc/samsung/fvmap.h>

#include "cal-if/cmucal.h"

#define OC_CPUCL0_MAX_KHZ	2210000U
#define OC_CPUCL1_MAX_KHZ	2704000U
#define OC_DSU_MAX_KHZ		1898000U
#define OC_GPU_MAX_KHZ		1053000U
#define OC_MIF_SAFE_MAX_KHZ	2093000U
#define OC_GPU_DEFAULT_LIT_MIN	1069000U
#define OC_GPU_COLS		8U
#define OC_GPU_MAX_ROWS		32U

#define OC_UV_PERCENT		(-2)

#define OC_THERMAL_DELTA_MC	5000
#define OC_SWITCH_ON_MAX_MC	82000
#define OC_CONTROL_MAX_MC	95000

static int oc_update_dt_u32_array(struct device_node *np, const char *name,
				  const u32 *vals, size_t count)
{
	struct property *prop;
	__be32 *be_vals;
	size_t i;
	int ret;

	prop = kzalloc(sizeof(*prop), GFP_KERNEL);
	if (!prop)
		return -ENOMEM;

	prop->name = kstrdup(name, GFP_KERNEL);
	be_vals = kcalloc(count, sizeof(*be_vals), GFP_KERNEL);
	if (!prop->name || !be_vals) {
		kfree(prop->name);
		kfree(be_vals);
		kfree(prop);
		return -ENOMEM;
	}

	for (i = 0; i < count; i++)
		be_vals[i] = cpu_to_be32(vals[i]);

	prop->value = be_vals;
	prop->length = count * sizeof(*be_vals);

	ret = of_update_property(np, prop);
	if (ret) {
		kfree(prop->name);
		kfree(be_vals);
		kfree(prop);
	}
	return ret;
}

/*
 * Prepend the SRAM-native 1053 MHz G3D step into the in-memory Mali DT node
 * before mali_kbase.ko probes, keeping MIF capped at the LPDDR4X 2093 MHz limit.
 */
static void __init oc_tune_gpu_dt(void)
{
	struct device_node *np;
	u32 gpu_max = 0;
	u32 size[2] = { 0, 0 };
	u32 *new_table = NULL;
	u32 rows, cols, total, new_total;
	u32 new_size[2];
	u32 max_val = OC_GPU_MAX_KHZ;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "arm,mali");
	if (!np)
		return;

	if (of_property_read_u32(np, "gpu_max_clock", &gpu_max) ||
	    gpu_max >= OC_GPU_MAX_KHZ)
		goto out;

	if (of_property_read_u32_array(np, "gpu_dvfs_table_size", size, 2))
		goto out;

	rows = size[0];
	cols = size[1];
	if (cols != OC_GPU_COLS || rows == 0 || rows >= OC_GPU_MAX_ROWS)
		goto out;

	total = rows * cols;
	new_total = (rows + 1) * cols;
	new_table = kcalloc(new_total, sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		goto out;

	if (of_property_read_u32_array(np, "gpu_dvfs_table",
				       new_table + cols, total))
		goto out;

	if (new_table[cols] >= OC_GPU_MAX_KHZ)
		goto out;

	/* New top GPU step: 1053 MHz with 82% down / 100% up thresholds */
	new_table[0] = OC_GPU_MAX_KHZ;
	new_table[1] = 82;
	new_table[2] = 100;
	new_table[3] = 5;
	new_table[4] = OC_MIF_SAFE_MAX_KHZ;
	new_table[5] = OC_GPU_DEFAULT_LIT_MIN;
	new_table[6] = 0;
	new_table[7] = 0;

	/* Allow 949 MHz to step up to 1053 MHz above 95% GPU load */
	if (new_table[cols + 2] >= 100)
		new_table[cols + 2] = 95;

	new_size[0] = rows + 1;
	new_size[1] = cols;

	ret = oc_update_dt_u32_array(np, "gpu_dvfs_table", new_table, new_total);
	if (!ret)
		ret = oc_update_dt_u32_array(np, "gpu_dvfs_table_size", new_size, 2);
	if (!ret)
		ret = oc_update_dt_u32_array(np, "gpu_max_clock", &max_val, 1);

	if (!ret)
		pr_info("exynos_oc_tune: enabled G3D %u kHz in DT\n", OC_GPU_MAX_KHZ);

out:
	kfree(new_table);
	of_node_put(np);
}

/* Lift top-app fclamp ceilings from 2002/2400 MHz to 2210/2704 MHz. */
static void __init oc_tune_fclamp_dt(void)
{
	struct device_node *np = NULL;
	u32 freqs[2];
	const u32 target[2] = { OC_CPUCL0_MAX_KHZ, OC_CPUCL1_MAX_KHZ };

	for_each_node_by_name(np, "fclamp") {
		if (of_property_read_u32_array(np, "max-freq", freqs, 2))
			continue;
		if (freqs[0] == 2002000U && freqs[1] == 2400000U)
			oc_update_dt_u32_array(np, "max-freq", target, 2);
	}
}

static void oc_apply_cmupmucal_uv(struct module *mod)
{
	unsigned int i;
	unsigned long addr;
	int *pm_table;
	bool applied = false;
	static const char * const param_names[] = {
		"margin_cpucl0", "margin_cpucl1", "margin_g3d",
	};
	static const int margin_ids[] = {
		MARGIN_CPUCL0, MARGIN_CPUCL1, MARGIN_G3D,
	};

	for (i = 0; i < mod->num_kp; i++) {
		unsigned int p;

		if (!mod->kp[i].name || !mod->kp[i].arg)
			continue;

		for (p = 0; p < ARRAY_SIZE(param_names); p++) {
			if (strcmp(mod->kp[i].name, param_names[p]) == 0) {
				WRITE_ONCE(*(int *)mod->kp[i].arg, OC_UV_PERCENT);
				applied = true;
			}
		}
	}

	addr = module_kallsyms_lookup_name("cmupmucal:percent_margin_table");
	if (addr) {
		pm_table = (int *)addr;
		for (i = 0; i < ARRAY_SIZE(margin_ids); i++) {
			if (margin_ids[i] < MAX_MARGIN_ID)
				WRITE_ONCE(pm_table[margin_ids[i]], OC_UV_PERCENT);
		}
	}

	if (applied)
		pr_info("exynos_oc_tune: applied %d%% undervolt to CPUCL0/CPUCL1/G3D\n",
			OC_UV_PERCENT);
}

static void oc_unlock_cmupmucal_vclk(void)
{
	unsigned long list_addr, size_addr;
	struct vclk *vclk_list;
	unsigned int vclk_size;

	list_addr = module_kallsyms_lookup_name("cmupmucal:acpm_vclk_list");
	size_addr = module_kallsyms_lookup_name("cmupmucal:acpm_vclk_size");
	if (!list_addr || !size_addr)
		return;

	vclk_list = (struct vclk *)list_addr;
	vclk_size = READ_ONCE(*(unsigned int *)size_addr);

	if (vclk_size < 9 ||
	    vclk_list[2].id != (ACPM_VCLK_TYPE | 2) ||
	    vclk_list[3].id != (ACPM_VCLK_TYPE | 3) ||
	    vclk_list[4].id != (ACPM_VCLK_TYPE | 4) ||
	    vclk_list[8].id != (ACPM_VCLK_TYPE | 8)) {
		pr_warn("exynos_oc_tune: unexpected vclk layout, skipping CPU/GPU unlock\n");
		return;
	}

	if (vclk_list[2].max_freq == OC_CPUCL0_MAX_KHZ &&
	    vclk_list[3].max_freq == OC_CPUCL1_MAX_KHZ &&
	    vclk_list[4].max_freq >= OC_DSU_MAX_KHZ &&
	    vclk_list[8].max_freq == OC_GPU_MAX_KHZ)
		return;

	if (vclk_list[2].max_freq < 1500000U || vclk_list[2].max_freq > OC_CPUCL0_MAX_KHZ ||
	    vclk_list[3].max_freq < 2000000U || vclk_list[3].max_freq > OC_CPUCL1_MAX_KHZ ||
	    vclk_list[8].max_freq < 800000U  || vclk_list[8].max_freq > OC_GPU_MAX_KHZ) {
		pr_warn("exynos_oc_tune: unexpected stock max_freq values, skipping unlock\n");
		return;
	}

	vclk_list[2].max_freq = OC_CPUCL0_MAX_KHZ;
	vclk_list[3].max_freq = OC_CPUCL1_MAX_KHZ;
	if (vclk_list[4].max_freq < OC_DSU_MAX_KHZ)
		vclk_list[4].max_freq = OC_DSU_MAX_KHZ;
	vclk_list[8].max_freq = OC_GPU_MAX_KHZ;

	pr_info("exynos_oc_tune: unlocked SRAM limits LITTLE=%u BIG=%u DSU=%u G3D=%u kHz\n",
		vclk_list[2].max_freq, vclk_list[3].max_freq,
		vclk_list[4].max_freq, vclk_list[8].max_freq);
}

static bool oc_bump_trip(struct thermal_zone_device *tz, int trip,
			 enum thermal_trip_type expected_type,
			 int min_cur, int max_cur, int hard_cap)
{
	enum thermal_trip_type type;
	int temp, target;

	if (tz->ops->get_trip_type &&
	    (tz->ops->get_trip_type(tz, trip, &type) || type != expected_type))
		return false;

	if (tz->ops->get_trip_temp(tz, trip, &temp))
		return false;

	if (temp < min_cur || temp > max_cur)
		return false;

	target = min(temp + OC_THERMAL_DELTA_MC, hard_cap);
	if (target == temp)
		return false;

	if (tz->ops->set_trip_temp(tz, trip, target))
		return false;

	return true;
}

static void oc_tune_thermal_zones(void)
{
	static const char * const zone_names[] = { "BIG", "LITTLE", "G3D" };
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(zone_names); i++) {
		struct thermal_zone_device *tz;
		bool changed = false;

		tz = thermal_zone_get_zone_by_name(zone_names[i]);
		if (IS_ERR(tz) || !tz->ops ||
		    !tz->ops->get_trip_temp || !tz->ops->set_trip_temp)
			continue;

		/* trip 1: switch_on (75C -> 80C), trip 2: control_temp (85/88C -> 90/93C) */
		changed |= oc_bump_trip(tz, 1, THERMAL_TRIP_ACTIVE,
					60000, 77000, OC_SWITCH_ON_MAX_MC);
		changed |= oc_bump_trip(tz, 2, THERMAL_TRIP_PASSIVE,
					75000, 90000, OC_CONTROL_MAX_MC);

		if (changed) {
			thermal_zone_device_update(tz, THERMAL_EVENT_UNSPECIFIED);
			pr_info("exynos_oc_tune: raised thermal trips (+5C) for %s\n",
				zone_names[i]);
		}
	}
}

static void oc_thermal_work_fn(struct work_struct *work)
{
	oc_tune_thermal_zones();
}

static DECLARE_DELAYED_WORK(oc_thermal_work, oc_thermal_work_fn);

static int oc_module_notify(struct notifier_block *nb,
			    unsigned long action, void *data)
{
	struct module *mod = data;

	if (!mod)
		return NOTIFY_DONE;

	if (strcmp(mod->name, "cmupmucal") == 0) {
		if (action == MODULE_STATE_COMING)
			oc_apply_cmupmucal_uv(mod);
		else if (action == MODULE_STATE_LIVE)
			oc_unlock_cmupmucal_vclk();
	} else if ((strcmp(mod->name, "exynos_acme") == 0 ||
		    strcmp(mod->name, "exynos_dsufreq") == 0 ||
		    strcmp(mod->name, "mali_kbase") == 0) &&
		   action == MODULE_STATE_COMING) {
		oc_unlock_cmupmucal_vclk();
	} else if (strcmp(mod->name, "exynos_thermal_v2") == 0 &&
		   action == MODULE_STATE_LIVE) {
		oc_tune_thermal_zones();
		schedule_delayed_work(&oc_thermal_work, 5 * HZ);
	}

	return NOTIFY_DONE;
}

static struct notifier_block oc_module_nb = {
	.notifier_call = oc_module_notify,
	.priority = INT_MAX,
};

static int __init exynos_oc_tune_init(void)
{
	int ret;

	oc_tune_gpu_dt();
	oc_tune_fclamp_dt();

	ret = register_module_notifier(&oc_module_nb);
	if (ret)
		pr_warn("exynos_oc_tune: register_module_notifier failed (%d)\n", ret);

	return 0;
}
late_initcall(exynos_oc_tune_init);
