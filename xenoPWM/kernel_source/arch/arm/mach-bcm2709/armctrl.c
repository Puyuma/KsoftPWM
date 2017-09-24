/*
 *  linux/arch/arm/mach-bcm2708/armctrl.c
 *
 *  Copyright (C) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/init.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/syscore_ops.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/of.h>

#include <asm/mach/irq.h>
#include <mach/hardware.h>
#include "armctrl.h"

/* For support of kernels >= 3.0 assume only one VIC for now*/
static unsigned int remap_irqs[(INTERRUPT_ARASANSDIO + 1) - INTERRUPT_JPEG] = {
	INTERRUPT_VC_JPEG,
	INTERRUPT_VC_USB,
	INTERRUPT_VC_3D,
	INTERRUPT_VC_DMA2,
	INTERRUPT_VC_DMA3,
	INTERRUPT_VC_I2C,
	INTERRUPT_VC_SPI,
	INTERRUPT_VC_I2SPCM,
	INTERRUPT_VC_SDIO,
	INTERRUPT_VC_UART,
	INTERRUPT_VC_ARASANSDIO
};

extern unsigned force_core;

static void armctrl_mask_irq(struct irq_data *d)
{
	static const unsigned int disables[4] = {
		ARM_IRQ_DIBL1,
		ARM_IRQ_DIBL2,
		ARM_IRQ_DIBL3,
		0
	};
	int i;
	if (d->irq >= FIQ_START) {
		writel(0, __io_address(ARM_IRQ_FAST));
	} else if (d->irq >= IRQ_ARM_LOCAL_CNTPSIRQ && d->irq < IRQ_ARM_LOCAL_CNTPSIRQ + 4) {
#if 1
		unsigned int data = (unsigned int)irq_get_chip_data(d->irq) - IRQ_ARM_LOCAL_CNTPSIRQ;
		for (i=0; i<4; i++) // i = raw_smp_processor_id(); //
		{
			unsigned int val =   readl(__io_address(ARM_LOCAL_TIMER_INT_CONTROL0 + 4*i));
			writel(val &~ (1 << data), __io_address(ARM_LOCAL_TIMER_INT_CONTROL0 + 4*i));
		}
#endif
	} else if (d->irq >= IRQ_ARM_LOCAL_MAILBOX0 && d->irq < IRQ_ARM_LOCAL_MAILBOX0 + 4) {
#if 0
		unsigned int data = (unsigned int)irq_get_chip_data(d->irq) - IRQ_ARM_LOCAL_MAILBOX0;
		for (i=0; i<4; i++) {
			unsigned int val = readl(__io_address(ARM_LOCAL_MAILBOX_INT_CONTROL0 + 4*i));
			writel(val &~ (1 << data), __io_address(ARM_LOCAL_MAILBOX_INT_CONTROL0 + 4*i));
		}
#endif
	} else if (d->irq >= ARM_IRQ1_BASE && d->irq < ARM_IRQ_LOCAL_BASE) {
		unsigned int data = (unsigned int)irq_get_chip_data(d->irq);
		writel(1 << (data & 0x1f), __io_address(disables[(data >> 5) & 0x3]));
	} else if (d->irq == INTERRUPT_ARM_LOCAL_PMU_FAST) {
		writel(0xf, __io_address(ARM_LOCAL_PM_ROUTING_CLR));
	} else { printk("%s: %d\n", __func__, d->irq); BUG(); }
}

static void armctrl_unmask_irq(struct irq_data *d)
{
	static const unsigned int enables[4] = {
		ARM_IRQ_ENBL1,
		ARM_IRQ_ENBL2,
		ARM_IRQ_ENBL3,
		0
	};
	int i;
	if (d->irq >= FIQ_START) {
		unsigned int data;
		if (force_core) {
			data = readl(__io_address(ARM_LOCAL_GPU_INT_ROUTING));
			data &= ~0xc;
			data |= ((force_core-1) << 2);
			writel(data, __io_address(ARM_LOCAL_GPU_INT_ROUTING));
		}
		else if (num_online_cpus() > 1) {
			data = readl(__io_address(ARM_LOCAL_GPU_INT_ROUTING));
			data &= ~0xc;
			data |= (1 << 2);
			writel(data, __io_address(ARM_LOCAL_GPU_INT_ROUTING));
		}
		/* Unmask in ARMCTRL block after routing it properly */
		data = (unsigned int)irq_get_chip_data(d->irq) - FIQ_START;
		writel(0x80 | data, __io_address(ARM_IRQ_FAST));
	} else if (d->irq >= IRQ_ARM_LOCAL_CNTPSIRQ && d->irq < IRQ_ARM_LOCAL_CNTPSIRQ + 4) {
#if 1
		unsigned int data = (unsigned int)irq_get_chip_data(d->irq) - IRQ_ARM_LOCAL_CNTPSIRQ;
		for (i=0; i<4; i++) // i = raw_smp_processor_id();
		{
			unsigned int val =  readl(__io_address(ARM_LOCAL_TIMER_INT_CONTROL0 + 4*i));
			writel(val | (1 << data), __io_address(ARM_LOCAL_TIMER_INT_CONTROL0 + 4*i));
		}
#endif
	} else if (d->irq >= IRQ_ARM_LOCAL_MAILBOX0 && d->irq < IRQ_ARM_LOCAL_MAILBOX0 + 4) {
#if 0
		unsigned int data = (unsigned int)irq_get_chip_data(d->irq) - IRQ_ARM_LOCAL_MAILBOX0;
		for (i=0; i<4; i++) {
			unsigned int val = readl(__io_address(ARM_LOCAL_MAILBOX_INT_CONTROL0 + 4*i));
			writel(val | (1 << data), __io_address(ARM_LOCAL_MAILBOX_INT_CONTROL0 + 4*i));
		}
#endif
	} else if (d->irq >= ARM_IRQ1_BASE && d->irq < ARM_IRQ_LOCAL_BASE) {
		if (force_core) {
			unsigned int data;
			data = readl(__io_address(ARM_LOCAL_GPU_INT_ROUTING));
			data &= ~0x3;
			data |= ((force_core-1) << 0);
			writel(data, __io_address(ARM_LOCAL_GPU_INT_ROUTING));
		}
		unsigned int data = (unsigned int)irq_get_chip_data(d->irq);
		writel(1 << (data & 0x1f), __io_address(enables[(data >> 5) & 0x3]));
	} else if (d->irq == INTERRUPT_ARM_LOCAL_PMU_FAST) {
		writel(0xf, __io_address(ARM_LOCAL_PM_ROUTING_SET));
	} else { printk("%s: %d\n", __func__, d->irq); BUG(); }
}

#ifdef CONFIG_OF

#define NR_IRQS_BANK0           21
#define NR_BANKS                4
#define IRQS_PER_BANK           32

/* from drivers/irqchip/irq-bcm2835.c */
static int armctrl_xlate(struct irq_domain *d, struct device_node *ctrlr,
        const u32 *intspec, unsigned int intsize,
        unsigned long *out_hwirq, unsigned int *out_type)
{
        if (WARN_ON(intsize != 2))
                return -EINVAL;

        if (WARN_ON(intspec[0] >= NR_BANKS))
                return -EINVAL;

        if (WARN_ON(intspec[1] >= IRQS_PER_BANK))
                return -EINVAL;

        if (WARN_ON(intspec[0] == 0 && intspec[1] >= NR_IRQS_BANK0))
                return -EINVAL;

        if (WARN_ON(intspec[0] == 3 && intspec[1] > 3 && intspec[1] != 5 && intspec[1] != 9))
                return -EINVAL;

	if (intspec[0] == 0)
		*out_hwirq = ARM_IRQ0_BASE + intspec[1];
	else if (intspec[0] == 1)
		*out_hwirq = ARM_IRQ1_BASE + intspec[1];
	else if (intspec[0] == 2)
		*out_hwirq = ARM_IRQ2_BASE + intspec[1];
	else
		*out_hwirq = ARM_IRQ_LOCAL_BASE + intspec[1];

	/* reverse remap_irqs[] */
	switch (*out_hwirq) {
	case INTERRUPT_VC_JPEG:
		*out_hwirq = INTERRUPT_JPEG;
		break;
	case INTERRUPT_VC_USB:
		*out_hwirq = INTERRUPT_USB;
		break;
	case INTERRUPT_VC_3D:
		*out_hwirq = INTERRUPT_3D;
		break;
	case INTERRUPT_VC_DMA2:
		*out_hwirq = INTERRUPT_DMA2;
		break;
	case INTERRUPT_VC_DMA3:
		*out_hwirq = INTERRUPT_DMA3;
		break;
	case INTERRUPT_VC_I2C:
		*out_hwirq = INTERRUPT_I2C;
		break;
	case INTERRUPT_VC_SPI:
		*out_hwirq = INTERRUPT_SPI;
		break;
	case INTERRUPT_VC_I2SPCM:
		*out_hwirq = INTERRUPT_I2SPCM;
		break;
	case INTERRUPT_VC_SDIO:
		*out_hwirq = INTERRUPT_SDIO;
		break;
	case INTERRUPT_VC_UART:
		*out_hwirq = INTERRUPT_UART;
		break;
	case INTERRUPT_VC_ARASANSDIO:
		*out_hwirq = INTERRUPT_ARASANSDIO;
		break;
	}

        *out_type = IRQ_TYPE_NONE;
        return 0;
}

static struct irq_domain_ops armctrl_ops = {
        .xlate = armctrl_xlate
};

void __init armctrl_dt_init(void)
{
	struct device_node *np;
	struct irq_domain *domain;

	np = of_find_compatible_node(NULL, NULL, "brcm,bcm2708-armctrl-ic");
	if (!np)
		return;

	domain = irq_domain_add_legacy(np, BCM2708_ALLOC_IRQS,
					IRQ_ARMCTRL_START, 0,
					&armctrl_ops, NULL);
        WARN_ON(!domain);
}
#else
void __init armctrl_dt_init(void) { }
#endif /* CONFIG_OF */

#if defined(CONFIG_PM)

/* for kernels 3.xx use the new syscore_ops apis but for older kernels use the sys dev class */

/* Static defines
 * struct armctrl_device - VIC PM device (< 3.xx)
 * @sysdev: The system device which is registered. (< 3.xx)
 * @irq: The IRQ number for the base of the VIC.
 * @base: The register base for the VIC.
 * @resume_sources: A bitmask of interrupts for resume.
 * @resume_irqs: The IRQs enabled for resume.
 * @int_select: Save for VIC_INT_SELECT.
 * @int_enable: Save for VIC_INT_ENABLE.
 * @soft_int: Save for VIC_INT_SOFT.
 * @protect: Save for VIC_PROTECT.
 */
struct armctrl_info {
	void __iomem *base;
	int irq;
	u32 resume_sources;
	u32 resume_irqs;
	u32 int_select;
	u32 int_enable;
	u32 soft_int;
	u32 protect;
} armctrl;

static int armctrl_suspend(void)
{
	return 0;
}

static void armctrl_resume(void)
{
	return;
}

/**
 * armctrl_pm_register - Register a VIC for later power management control
 * @base: The base address of the VIC.
 * @irq: The base IRQ for the VIC.
 * @resume_sources: bitmask of interrupts allowed for resume sources.
 *
 * For older kernels (< 3.xx) do -
 * Register the VIC with the system device tree so that it can be notified
 * of suspend and resume requests and ensure that the correct actions are
 * taken to re-instate the settings on resume.
 */
static void __init armctrl_pm_register(void __iomem * base, unsigned int irq,
				       u32 resume_sources)
{
	armctrl.base = base;
	armctrl.resume_sources = resume_sources;
	armctrl.irq = irq;
}

static int armctrl_set_wake(struct irq_data *d, unsigned int on)
{
	unsigned int off = d->irq & 31;
	u32 bit = 1 << off;

	if (!(bit & armctrl.resume_sources))
		return -EINVAL;

	if (on)
		armctrl.resume_irqs |= bit;
	else
		armctrl.resume_irqs &= ~bit;

	return 0;
}

#else
static inline void armctrl_pm_register(void __iomem * base, unsigned int irq,
				       u32 arg1)
{
}

#define armctrl_suspend NULL
#define armctrl_resume NULL
#define armctrl_set_wake NULL
#endif /* CONFIG_PM */

static struct syscore_ops armctrl_syscore_ops = {
	.suspend = armctrl_suspend,
	.resume = armctrl_resume,
};

/**
 * armctrl_syscore_init - initicall to register VIC pm functions
 *
 * This is called via late_initcall() to register
 * the resources for the VICs due to the early
 * nature of the VIC's registration.
*/
static int __init armctrl_syscore_init(void)
{
	register_syscore_ops(&armctrl_syscore_ops);
	return 0;
}

late_initcall(armctrl_syscore_init);

static struct irq_chip armctrl_chip = {
	.name = "ARMCTRL",
	.irq_ack = NULL,
	.irq_mask = armctrl_mask_irq,
	.irq_unmask = armctrl_unmask_irq,
	.irq_set_wake = armctrl_set_wake,
#ifdef CONFIG_IPIPE
	.irq_hold = armctrl_mask_irq,
	.irq_release = armctrl_unmask_irq,
#endif
};

/**
 * armctrl_init - initialise a vectored interrupt controller
 * @base: iomem base address
 * @irq_start: starting interrupt number, must be muliple of 32
 * @armctrl_sources: bitmask of interrupt sources to allow
 * @resume_sources: bitmask of interrupt sources to allow for resume
 */
int __init armctrl_init(void __iomem * base, unsigned int irq_start,
			u32 armctrl_sources, u32 resume_sources)
{
	unsigned int irq;

	for (irq = 0; irq < BCM2708_ALLOC_IRQS; irq++) {
		unsigned int data = irq;
		if (irq >= INTERRUPT_JPEG && irq <= INTERRUPT_ARASANSDIO)
			data = remap_irqs[irq - INTERRUPT_JPEG];
		if (irq >= IRQ_ARM_LOCAL_CNTPSIRQ && irq <= IRQ_ARM_LOCAL_TIMER) {
			irq_set_percpu_devid(irq);
			irq_set_chip_and_handler(irq, &armctrl_chip, handle_percpu_devid_irq);
			set_irq_flags(irq, IRQF_VALID | IRQF_NOAUTOEN);
                } else {
			irq_set_chip_and_handler(irq, &armctrl_chip, handle_level_irq);
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
                }
		irq_set_chip_data(irq, (void *)data);
	}

	armctrl_pm_register(base, irq_start, resume_sources);
	init_FIQ(FIQ_START);
	armctrl_dt_init();
	return 0;
}
