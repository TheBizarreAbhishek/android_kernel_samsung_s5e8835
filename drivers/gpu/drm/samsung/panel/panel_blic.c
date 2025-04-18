// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>

#include "panel_drv.h"
#include "panel_blic.h"
#include "panel_i2c.h"
#include "panel_debug.h"

#define BLIC_DT_NODE_NAME "blic"

static int panel_blic_do_seq(struct panel_blic_dev *blic, char *seqname);

static struct panel_blic_ops panel_blic_ops = {
	.do_seq = panel_blic_do_seq,
	.execute_power_ctrl = panel_blic_power_ctrl_execute,
};

__visible_for_testing struct panel_device *to_panel_drv(struct panel_blic_dev *blic_dev)
{
	if (!blic_dev)
		return NULL;

	return container_of(blic_dev, struct panel_device, blic_dev[blic_dev->id]);
}

struct seqinfo *find_blic_seq(struct panel_blic_dev *blic, char *seqname)
{
	struct pnobj *pnobj;

	if (!blic) {
		panel_err("blic is null\n");
		return NULL;
	}

	if (list_empty(&blic->seq_list))
		return NULL;

	pnobj = pnobj_find_by_name(&blic->seq_list, seqname);
	if (!pnobj)
		return NULL;

	return pnobj_container_of(pnobj, struct seqinfo);
}

static bool panel_blic_check_seqtbl_exist(struct panel_blic_dev *blic, char *seqname)
{
	struct seqinfo *seq;

	seq = find_blic_seq(blic, seqname);
	if (!seq)
		return false;

	if (!is_valid_sequence(seq))
		return false;

	return true;
}

static int panel_blic_do_seq_nolock(struct panel_blic_dev *blic, char *seqname)
{
	struct seqinfo *seq;

	if (!blic) {
		panel_err("blic is null\n");
		return -EINVAL;
	}

	seq = find_blic_seq(blic, seqname);
	if (!seq)
		return -EINVAL;

	/*
	 * @ i2c_dev_selected
	 *
	 * There can be many i2c drivers,
	 * "panel_do_seqtbl" don't know who should do among them.
	 * So, leave a selected i2c_dev pointer for hint.
	 * .i2c_dev_selected should be protected in panel->op_lock.
	 */

#ifdef CONFIG_USDM_BLIC_I2C
	if (blic->i2c_dev) {
		struct panel_device *panel =
			to_panel_drv(blic);

		if (!panel) {
			panel_err("panel is null\n");
			return -EINVAL;
		}

		panel->i2c_dev_selected = blic->i2c_dev;
	}
#endif

	return execute_sequence_nolock(to_panel_drv(blic), seq);
}

static int panel_blic_do_seq(struct panel_blic_dev *blic, char *seqname)
{
	struct panel_device *panel;
	int ret = 0;

	if (!blic) {
		panel_err("blic is null.\n");
		return -EINVAL;
	}

	if (!seqname) {
		panel_err("seqname is null.\n");
		return -EINVAL;
	}

	panel = to_panel_drv(blic);
	if (!panel) {
		panel_err("panel is null.\n");
		return -EINVAL;
	}

	panel_mutex_lock(&panel->op_lock);
	ret = panel_blic_do_seq_nolock(blic, seqname);
	if (ret < 0) {
		panel_err("failed to run sequence(%s)\n", seqname);
		ret = -EIO;
		goto do_exit;
	}

do_exit:
	panel_mutex_unlock(&panel->op_lock);

	return ret;
}

int panel_blic_brightness(struct panel_device *panel, bool need_lock)
{
	int ret = 0;
	int i;

	if (need_lock)
		panel_mutex_lock(&panel->op_lock);

	for (i = 0; i < panel->nr_blic_dev; i++) {
		if (panel_blic_check_seqtbl_exist(&panel->blic_dev[i], PANEL_BLIC_I2C_BRIGHTNESS_SEQ)) {
			ret = panel_blic_do_seq_nolock(&panel->blic_dev[i], PANEL_BLIC_I2C_BRIGHTNESS_SEQ);
			if (ret) {
				panel_err("panel_blic_do_seq is failed.(%d)\n", ret);
				goto panel_blic_brightness_exit;
			}
		}
	}

panel_blic_brightness_exit:
	if (need_lock)
		panel_mutex_unlock(&panel->op_lock);

	return ret;
}

bool panel_blic_power_ctrl_exists(struct panel_blic_dev *blic, const char *name)
{
	if (!blic || !name) {
		panel_err("invalid arg");
		return false;
	}

	if (!blic->np) {
		panel_err("of node is empty");
		return false;
	}

	if (!blic->np->name) {
		panel_err("np name is empty");
		return false;
	}

	return panel_power_ctrl_exists(to_panel_drv(blic), blic->np->name, name);
}

int panel_blic_power_ctrl_execute(struct panel_blic_dev *blic, const char *name)
{
	if (!blic || !name) {
		panel_err("invalid arg\n");
		return -EINVAL;
	}

	if (!blic->np) {
		panel_err("of node is empty");
		return -ENODEV;
	}

	if (!blic->np->name) {
		panel_err("np name is empty");
		return -ENODEV;
	}

	return panel_power_ctrl_execute(to_panel_drv(blic), blic->np->name, name);
}

/* enable/disable regulator */
int panel_blic_regulator_enable(struct regulator_dev *rdev)
{
	struct panel_blic_dev *blic;
	int ret = 0;

	panel_info("\n");

	if (!rdev) {
		panel_err("rdev is null.\n");
		return -EINVAL;
	}

	blic = rdev->reg_data;
	if (!blic) {
		panel_err("blic is null.\n");
		return -EINVAL;
	}

	if (blic->enable) {
		panel_info("already enabled\n");
		return 0;
	}

	if (!blic->ops) {
		panel_info("blic ops is null\n");
		return -EINVAL;
	}

	if (!blic->ops->do_seq) {
		panel_info("do_seq is null\n");
		return -EINVAL;
	}

	if (!blic->ops->execute_power_ctrl) {
		panel_info("execute_power_ctrl is null\n");
		return -EINVAL;
	}

	if (panel_blic_power_ctrl_exists(blic, "panel_blic_pre_on"))
		ret |= blic->ops->execute_power_ctrl(blic, "panel_blic_pre_on");

	if (panel_blic_check_seqtbl_exist(blic, PANEL_BLIC_I2C_ON_SEQ))
		ret |= blic->ops->do_seq(blic, PANEL_BLIC_I2C_ON_SEQ);

	if (panel_blic_power_ctrl_exists(blic, "panel_blic_post_on"))
		ret |= blic->ops->execute_power_ctrl(blic, "panel_blic_post_on");

	if (ret) {
		panel_err("failed.(%d)\n", ret);
		return -EINVAL;
	}

	blic->enable = 1;

	return 0;
}

__visible_for_testing int panel_blic_regulator_disable(struct regulator_dev *rdev)
{
	struct panel_blic_dev *blic;
	int ret = 0;

	if (!rdev) {
		panel_err("rdev is null.\n");
		return -EINVAL;
	}

	blic = rdev->reg_data;
	if (!blic) {
		panel_err("blic is null.\n");
		return -EINVAL;
	}

	panel_info("\n");

	if (!blic->ops) {
		panel_info("blic ops is null\n");
		return -EINVAL;
	}

	if (!blic->ops->do_seq) {
		panel_info("do_seq is null\n");
		return -EINVAL;
	}

	if (!blic->ops->execute_power_ctrl) {
		panel_info("execute_power_ctrl is null\n");
		return -EINVAL;
	}

	if (!blic->enable) {
		panel_info("already disabled\n");
		return 0;
	}

	if (panel_blic_power_ctrl_exists(blic, "panel_blic_pre_off"))
		ret |= blic->ops->execute_power_ctrl(blic, "panel_blic_pre_off");

	if (panel_blic_check_seqtbl_exist(blic, PANEL_BLIC_I2C_OFF_SEQ))
		ret |= blic->ops->do_seq(blic, PANEL_BLIC_I2C_OFF_SEQ);

	if (panel_blic_power_ctrl_exists(blic, "panel_blic_post_off"))
		ret |= blic->ops->execute_power_ctrl(blic, "panel_blic_post_off");

	if (ret) {
		panel_err("failed.(%d)\n", ret);
		return -EINVAL;
	}

	blic->enable = 0;

	return 0;
}

static int panel_blic_is_enabled_default(struct regulator_dev *rdev)
{
	struct panel_blic_dev *blic;

	if (!rdev) {
		panel_err("rdev is null.\n");
		return -EINVAL;
	}

	blic = rdev->reg_data;
	if (!blic) {
		panel_err("blic is null.\n");
		return -EINVAL;
	}

	return blic->enable;
}

/* get/set regulator voltage */
static int panel_blic_set_voltage_default(struct regulator_dev *rdev, int min_uV, int max_uV, unsigned *selector)
{
	if (!rdev) {
		panel_err("rdev is null.\n");
		return -EINVAL;
	}

	panel_info("PANEL:INFO:%s\n", __func__);

	return 0;
}

static int panel_blic_get_voltage_default(struct regulator_dev *rdev)
{
	struct panel_blic_dev *blic;

	if (!rdev) {
		panel_err("rdev is null.\n");
		return -EINVAL;
	}

	blic = rdev->reg_data;

	panel_info("PANEL:INFO:%s\n", __func__);

	return blic->enable ? rdev->constraints->max_uV : 0;
}


static struct regulator_ops ddi_blic_regulator_ops = {
	.is_enabled	= panel_blic_is_enabled_default,
	.enable		= panel_blic_regulator_enable,
	.disable		= panel_blic_regulator_disable,
	.set_voltage	= panel_blic_set_voltage_default,
	.get_voltage	= panel_blic_get_voltage_default,
};

static struct regulator_desc ddi_blic_regulator_desc = {
	.name = "ddi-blic",
	.id = 0,
	.ops = &ddi_blic_regulator_ops,
	.type = REGULATOR_VOLTAGE,
	.owner = THIS_MODULE,
};

static struct regulator_desc ddi_blic_2_regulator_desc = {
	.name = "ddi-blic-2",
	.id = 0,
	.ops = &ddi_blic_regulator_ops,
	.type = REGULATOR_VOLTAGE,
	.owner = THIS_MODULE,
};

static struct regulator_desc ddi_buck_booster_regulator_desc = {
	.name = "ddi-buck-booster",
	.id = 0,
	.ops = &ddi_blic_regulator_ops,
	.type = REGULATOR_VOLTAGE,
	.owner = THIS_MODULE,
};

static struct regulator_desc *panel_blic_regulator_list[] = {
	&ddi_blic_regulator_desc,
	&ddi_blic_2_regulator_desc,
	&ddi_buck_booster_regulator_desc,
};

__visible_for_testing struct regulator_desc *panel_blic_find_regulator_desc(const char *name)
{
	int i = 0;

	if (!name) {
		panel_err("name is null.\n");
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(panel_blic_regulator_list); i++) {
		if (!strcmp(name, panel_blic_regulator_list[i]->name))
			return panel_blic_regulator_list[i];
	}

	return NULL;
}

#if !defined(CONFIG_UML)
static int panel_blic_regulator_register(struct panel_blic_dev *blic_dev)
{
	struct regulator_config config = {0, };
	struct regulator_dev *rdev;
	struct regulator_desc *desc;

	if (!blic_dev) {
		panel_err("blic_dev is null.\n");
		return -EINVAL;
	}

	desc = panel_blic_find_regulator_desc(blic_dev->regulator_desc_name);
	if (!desc) {
		panel_err("failed to find regulator desc.(%s)\n", blic_dev->regulator_desc_name);
		return -EINVAL;
	}

	config.driver_data = blic_dev;
	config.of_node = blic_dev->regulator_node;
	config.dev = blic_dev->dev;
	config.init_data = of_get_regulator_init_data(blic_dev->dev, blic_dev->regulator_node, desc);

	if (blic_dev->regulator_boot_on) {
		panel_info("%s (regulator-boot-on)\n", desc->name);
		blic_dev->enable = 1;
	}

	rdev = devm_regulator_register(blic_dev->dev, desc, &config);
	if (IS_ERR(rdev)) {
		panel_err("rdev is err.(%s)(%ld)\n", desc->name, PTR_ERR(rdev));
		return -EINVAL;
	}

	panel_info("(%s) is registered\n", desc->name);

	return 0;
}
#else
static inline int panel_blic_regulator_register(struct panel_blic_dev *blic_dev)
{
	return 0;
}
#endif

#ifdef CONFIG_USDM_BLIC_I2C
__visible_for_testing struct panel_i2c_dev *panel_blic_find_i2c_drv(struct panel_blic_dev *blic_dev, struct panel_device *panel)
{
	u8 i2c_read_buf[2] = { 0, };
	int i = 0;

	if (!blic_dev) {
		panel_err("blic_dev is null.\n");
		return NULL;
	}

	if (!panel) {
		panel_err("panel is null.\n");
		return NULL;
	}

	/*
	 * Find I2C for BLIC
	 * 1. Check reg (slave addr).
	 * 2. Check if it is used.
	 * 3. Read IC ID register.
	 */

	i2c_read_buf[0] = blic_dev->i2c_match[0];

	for (i = 0; i < panel->nr_i2c_dev; i++) {
		if (panel->i2c_dev[i].reg != blic_dev->i2c_reg)
			continue;

		if (panel->i2c_dev[i].use)
			continue;

		panel->i2c_dev[i].ops->rx(&panel->i2c_dev[i], i2c_read_buf, 2);

		if ((i2c_read_buf[1] & blic_dev->i2c_match[1]) == blic_dev->i2c_match[2]) {
			panel_info("panel_i2c found. blic id:(0x%02X) mask:(0x%02X) id&mask:(0x%02X) val:(0x%02X)\n",
				i2c_read_buf[1], blic_dev->i2c_match[1],
				(i2c_read_buf[1]&blic_dev->i2c_match[1]), blic_dev->i2c_match[2]);

			return &panel->i2c_dev[i];
		}
	}

	return NULL;
}
#endif

static int of_get_panel_blic(struct panel_blic_dev *blic_dev, struct device_node *np)
{
	struct device_node *gpio_np, *gpios_np, *power_np, *seq_np;
	struct property *pp;
	struct panel_device *panel;
	struct panel_blic_dev *temp;
	struct panel_gpio gpio;
	struct panel_gpio *p_gpio;
	struct panel_power_ctrl *p_seq;
	int id = 0;
	int ret = 0;

	if (!blic_dev) {
		panel_err("blic_dev is null.\n");
		return -EINVAL;
	}

	if (!np) {
		panel_err("np is null.\n");
		return -EINVAL;
	}

	panel = to_panel_drv(blic_dev);

	if (!panel) {
		panel_err("panel is null.\n");
		return -EINVAL;
	}

	if (panel->nr_blic_dev >= PANEL_BLIC_MAX) {
		pr_err("i2c_dev array is full. (%d)\n",
				panel->nr_blic_dev);
		return -EINVAL;
	}

	temp = kzalloc(sizeof(*temp), GFP_KERNEL);

	id = panel->nr_blic_dev;

	temp->id = id;
	temp->np = np;
	temp->ops = &panel_blic_ops;

	/* name */
	ret = of_property_read_string(np, "name", &temp->name);
	if (ret < 0) {
		panel_err("name is null.\n");
		ret = -EINVAL;
		goto err;
	}
	/* i2c info */
	ret |= of_property_read_u32_array(np, "i2c,match", temp->i2c_match, 3);
	ret |= of_property_read_u32(np, "i2c,reg", &temp->i2c_reg);
	if (ret < 0) {
		panel_err("i2c is null.\n");
		ret = -EINVAL;
		goto err;
	}

	/* regulator info */
	temp->regulator_node = of_parse_phandle(temp->np, "regulator", 0);
	if (!temp->regulator_node) {
		panel_err("i2c is null.\n");
		ret = -EINVAL;
		goto err;
	}
	temp->regulator_boot_on = of_property_read_bool(temp->regulator_node, "regulator-boot-on");

	ret = of_property_read_string(temp->regulator_node, "regulator-name", &temp->regulator_name);
	if (ret < 0) {
		panel_err("regulator name is null.\n");
		ret = -EINVAL;
		goto err;
	}

	ret = of_property_read_string(temp->regulator_node, "regulator-desc-name", &temp->regulator_desc_name);
	if (ret < 0) {
		panel_err("regulator desc name is null.\n");
		ret = -EINVAL;
		goto err;
	}

	/* gpio info */
	gpios_np = of_get_child_by_name(np, "gpios");
	if (gpios_np) {
		for_each_child_of_node(gpios_np, gpio_np) {
			memset(&gpio, 0, sizeof(gpio));
			if (of_get_panel_gpio(gpio_np, &gpio)) {
				panel_err("failed to get gpio %s\n", gpio_np->name);
				break;
			}
			p_gpio = panel_gpio_list_add(panel, &gpio);
			if (!p_gpio) {
				panel_err("failed to add gpio list %s\n", gpio.name);
				break;
			}
		}
		of_node_put(gpios_np);
	} else
		panel_err("'gpios' node not found\n");

	/* power ctrl info */
	power_np = of_get_child_by_name(np, "power_ctrl");
	if (!power_np) {
		panel_err("'power_ctrl' node not found\n");
		ret = -EINVAL;
		goto exit;
	}

	seq_np = of_get_child_by_name(power_np, "sequences");
	if (!seq_np) {
		panel_err("'sequences' node not found\n");
		ret = -EINVAL;
		goto exit_power;
	}

	for_each_property_of_node(seq_np, pp) {
		if (!strcmp(pp->name, "name") || !strcmp(pp->name, "phandle"))
			continue;
		p_seq = kzalloc(sizeof(struct panel_power_ctrl), GFP_KERNEL);
		if (!p_seq) {
			ret = -ENOMEM;
			goto exit_power;
		}
		p_seq->dev_name = np->name;
		p_seq->name = pp->name;
		ret = of_get_panel_power_ctrl(panel, seq_np, pp->name, p_seq);
		if (ret < 0) {
			panel_err("failed to get power_ctrl %s\n", pp->name);
			break;
		}
		list_add_tail(&p_seq->head, &panel->power_ctrl_list);
		panel_info("power_ctrl '%s' initialized\n", p_seq->name);
	}
	of_node_put(seq_np);
exit_power:
	of_node_put(power_np);
exit:

#ifdef CONFIG_USDM_BLIC_I2C
	if (temp->i2c_reg) {
		temp->i2c_dev = panel_blic_find_i2c_drv(temp, panel);

		if (!temp->i2c_dev) {
			/*
			 * In most cases, it is not error.
			 * ex. Already i2c driver is occupied by a dualization device.
			 * The device doesn't need to probe anymore.
			 */

			panel_warn("cancel.(%s).\n", temp->name);
			kfree(temp);
			return ret;
		}

		temp->i2c_dev->use = true;
		temp->dev = &temp->i2c_dev->client->dev;
	}
#endif
	panel->nr_blic_dev++;

	memcpy(&blic_dev[id], temp, sizeof(struct panel_blic_dev));
	kfree(temp);

	blic_dev[id].id = id;

	panel_info("(%s) is registered(nr: %d).\n", blic_dev[id].name, panel->nr_blic_dev);

	return ret;
err:
	kfree(temp);
	return ret;
}

static int panel_blic_get_blic(struct panel_blic_dev *blic_dev, struct device_node *np)
{
	struct device_node *blic_np;
	int ret = 0;
	int id = 0;

	if (!blic_dev) {
		panel_err("blic_dev is null.\n");
		return -EINVAL;
	}

	if (!np) {
		panel_err("blic node is null.\n");
		return -EINVAL;
	}

	for_each_child_of_node(np, blic_np) {
		if (of_get_panel_blic(blic_dev, blic_np)) {
			panel_err("failed to get panel_blic %s\n", blic_np->name);
			ret = -EINVAL;
			break;
		}
		id++;
	}

	return ret;
}

__visible_for_testing int panel_blic_get_blic_regulator(struct panel_blic_dev *blic_dev)
{
	struct panel_device *panel;
	int ret;

	if (!blic_dev) {
		panel_err("blic_dev is null.\n");
		return -EINVAL;
	}

	panel = to_panel_drv(blic_dev);
	if (!panel) {
		panel_err("panel is null.\n");
		return -EINVAL;
	}

	ret = panel_blic_regulator_register(blic_dev);
	if (ret < 0) {
		panel_err("failed to register blic regulator(%s)\n",
				blic_dev->regulator_name);
		return ret;
	}

	return 0;
}

int panel_blic_prepare(struct panel_device *panel, struct common_panel_info *info)
{
	int i, j;
	struct blic_data *blic_data;
	struct panel_blic_dev *blic_dev;

	if (!panel)
		return -EINVAL;

	if (!info)
		return -EINVAL;

	if (!info->blic_data_tbl) {
		panel_err("blic_data_tbl is null\n");
		return -EINVAL;
	}

	for (i = 0; i < panel->nr_blic_dev; i++) {
		blic_dev = &panel->blic_dev[i];
		blic_dev->blic_data_tbl = info->blic_data_tbl[i];
		blic_data = info->blic_data_tbl[i];

		if (!blic_data) {
			panel_err("blic_data_tbl[%d] is null.\n", i);
			return 0;
		}

		for (j = 0; j < blic_data->nr_seqtbl; j++) {
			if (!is_valid_sequence(&blic_data->seqtbl[j]))
				continue;

			list_add_tail(get_pnobj_list(&blic_data->seqtbl[j].base),
					&blic_dev->seq_list);
		}
	}

	return 0;
}

int panel_blic_unprepare(struct panel_device *panel)
{
	struct panel_blic_dev *blic_dev;
	struct pnobj *pos, *next;
	int i;

	if (!panel)
		return -EINVAL;

	for (i = 0; i < panel->nr_blic_dev; i++) {
		blic_dev = &panel->blic_dev[i];

		if (!blic_dev->blic_data_tbl)
			continue;

		list_for_each_entry_safe(pos, next, &blic_dev->seq_list, list)
			list_del(&pos->list);
	}

	return 0;
}

int panel_blic_probe(struct panel_device *panel)
{
	struct panel_blic_dev *blic_dev;
	struct device_node *np;
	int i, ret = 0;

	if (!panel) {
		panel_err("panel is null.\n");
		return -EINVAL;
	}

	np = of_parse_phandle(panel->dev->of_node, BLIC_DT_NODE_NAME, 0);
	if (!np) {
		panel_err("failed to get blic DT node.\n");
		return -EINVAL;
	}

	blic_dev = panel->blic_dev;
	ret = panel_blic_get_blic(blic_dev, np);
	of_node_put(np);
	if (ret) {
		panel_err("failed to get blic.\n");
		return ret;
	}

	for (i = 0; i < panel->nr_blic_dev; i++) {
		blic_dev = &panel->blic_dev[i];

		ret = panel_blic_get_blic_regulator(blic_dev);
		if (ret) {
			panel_err("failed to get blic regulator.\n");
			return ret;
		}

		INIT_LIST_HEAD(&blic_dev->seq_list);
	}

	return ret;
}

MODULE_DESCRIPTION("blic driver for panel");
MODULE_LICENSE("GPL");
