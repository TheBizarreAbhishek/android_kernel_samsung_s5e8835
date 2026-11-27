// SPDX-License-Identifier: GPL-2.0
/*
 * Temporary read-only procfs inspector (/proc/fvmap_sram) for Exynos 1380
 * ACPM SRAM and copied RAM FVMap tables.
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <soc/samsung/fvmap.h>

#define FVMAP_DUMP_SIZE		SZ_8K
#define FVMAP_STEP_UV		6250U
#define FVMAP_MAX_DOMAINS	32U

static const char * const s5e8835_domain_names[] = {
	"MIF", "INT", "CPUCL0", "CPUCL1", "DSU",
	"M2M", "DISP", "AUD", "G3D", "CP_CPU",
	"CP", "GNSS", "CP_MCW", "NPU", "CAM",
	"CSIS", "ISP", "MFC", "WLBT", "INTSCI",
};

static const char *fvmap_domain_name(unsigned int idx)
{
	if (idx < ARRAY_SIZE(s5e8835_domain_names))
		return s5e8835_domain_names[idx];
	return "UNKNOWN";
}

static void __iomem *lookup_iomem_ptr(const char *sym_name)
{
	unsigned long addr;
	void __iomem **ptr;

	addr = module_kallsyms_lookup_name(sym_name);
	if (!addr)
		return NULL;

	ptr = (void __iomem **)addr;
	return READ_ONCE(*ptr);
}

static unsigned int lookup_domain_count(void)
{
	unsigned long addr;
	unsigned int *size_ptr;
	unsigned int count = ARRAY_SIZE(s5e8835_domain_names);

	addr = module_kallsyms_lookup_name("cmupmucal:acpm_vclk_size");
	if (addr) {
		size_ptr = (unsigned int *)addr;
		count = READ_ONCE(*size_ptr);
	}

	return min_t(unsigned int, count,
		     min_t(unsigned int, FVMAP_MAX_DOMAINS,
			   FVMAP_DUMP_SIZE / sizeof(struct fvmap_header)));
}

static void fvmap_dump_region(struct seq_file *m, const char *label,
			      const void __iomem *src, bool is_iomem,
			      unsigned int domain_count)
{
	const struct fvmap_header *headers;
	u8 *buf;
	unsigned int i, j;

	seq_printf(m, "=== %s (%pK) ===\n", label, src);
	if (!src) {
		seq_puts(m, "  (not mapped / NULL)\n");
		return;
	}

	buf = kmalloc(FVMAP_DUMP_SIZE, GFP_KERNEL);
	if (!buf) {
		seq_puts(m, "  (kmalloc failed)\n");
		return;
	}

	if (is_iomem)
		memcpy_fromio(buf, src, FVMAP_DUMP_SIZE);
	else
		memcpy(buf, (const void __force *)src, FVMAP_DUMP_SIZE);

	headers = (const struct fvmap_header *)buf;

	for (i = 0; i < domain_count; i++) {
		const struct fvmap_header *hdr = &headers[i];
		const struct rate_volt_header *rvh;
		size_t end_off;

		if (!hdr->num_of_lv)
			continue;

		end_off = (size_t)hdr->o_ratevolt +
			  (size_t)hdr->num_of_lv * sizeof(struct rate_volt);
		if (hdr->o_ratevolt >= FVMAP_DUMP_SIZE || end_off > FVMAP_DUMP_SIZE) {
			seq_printf(m, "domain[%u] %s: invalid o_ratevolt=0x%x num_of_lv=%u\n",
				   i, fvmap_domain_name(i), hdr->o_ratevolt, hdr->num_of_lv);
			continue;
		}

		seq_printf(m,
			   "domain[%u] %-8s id=0x%02x lv=%2u init=%u min=%u max=%u\n",
			   i, fvmap_domain_name(i), hdr->domain_id, hdr->num_of_lv,
			   hdr->init_rate, hdr->min_rate, hdr->max_rate);

		rvh = (const struct rate_volt_header *)(buf + hdr->o_ratevolt);
		for (j = 0; j < hdr->num_of_lv; j++) {
			seq_printf(m, "  lv[%02u]: %7u kHz, %7u uV\n",
				   j, rvh->table[j].rate,
				   rvh->table[j].volt * FVMAP_STEP_UV);
		}
	}

	kfree(buf);
}

static int fvmap_sram_show(struct seq_file *m, void *v)
{
	void __iomem *sram_base;
	void __iomem *ram_base;
	unsigned int domain_count;

	sram_base = lookup_iomem_ptr("cmupmucal:sram_fvmap_base");
	if (!sram_base)
		sram_base = lookup_iomem_ptr("exynos_acpm:fvmap_base_address");

	ram_base = lookup_iomem_ptr("cmupmucal:fvmap_base");
	domain_count = lookup_domain_count();

	fvmap_dump_region(m, "ACPM SRAM (sram_fvmap_base)", sram_base, true,
			  domain_count);
	fvmap_dump_region(m, "Kernel RAM (fvmap_base)", ram_base, false,
			  domain_count);
	return 0;
}

static int __init fvmap_dump_init(void)
{
	if (!proc_create_single("fvmap_sram", 0400, NULL, fvmap_sram_show))
		pr_warn("exynos_fvmap_dump: failed to create /proc/fvmap_sram\n");
	return 0;
}
late_initcall(fvmap_dump_init);
