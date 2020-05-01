#define DEBUG 1
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <asm/exception.h>

#include <soc/at91/at91sam9_ddrsdr.h>

#define AT91_AIC5_SSR   0x0
#define AT91_AIC5_INTSEL_MSK  (0x7f << 0)

#define AT91_AIC5_SMR     0x4

#define AT91_AIC5_SVR     0x8
#define AT91_AIC5_IVR     0x10
#define AT91_AIC5_FVR     0x14
#define AT91_AIC5_ISR     0x18

#define AT91_AIC5_IPR0      0x20
#define AT91_AIC5_IPR1      0x24
#define AT91_AIC5_IPR2      0x28
#define AT91_AIC5_IPR3      0x2c
#define AT91_AIC5_IMR     0x30
#define AT91_AIC5_CISR      0x34

#define AT91_AIC5_IECR      0x40
#define AT91_AIC5_IDCR      0x44
#define AT91_AIC5_ICCR      0x48
#define AT91_AIC5_ISCR      0x4c
#define AT91_AIC5_EOICR     0x38
#define AT91_AIC5_SPU     0x3c
#define AT91_AIC5_DCR     0x6c

#define AT91_AIC5_FFER      0x50
#define AT91_AIC5_FFDR      0x54
#define AT91_AIC5_FFSR      0x58

#define AT91_SHDW_CR	0x00		/* Shut Down Control Register */
#define AT91_SHDW_SHDW		BIT(0)			/* Shut Down command */
#define AT91_SHDW_KEY		(0xa5UL << 24)		/* KEY Password */

static void __iomem *aic5_base;
static struct gpio_desc *pio_out;

struct shdwc {
	const struct shdwc_config *cfg;
	struct clk *sclk;
	void __iomem *shdwc_base;
	void __iomem *mpddrc_base;
	void __iomem *pmc_base;
};

static struct shdwc *at91_shdwc;

static irqreturn_t irq_pin_interrupt(int irq, void *data)
{
	gpiod_set_value(pio_out, 1);

	asm volatile(
		/* Align to cache lines */
		".balign 32\n\t"

		/* Ensure AT91_SHDW_CR is in the TLB by reading it */
		"	ldr	r6, [%2, #" __stringify(AT91_SHDW_CR) "]\n\t"

		/* Power down SDRAM0 */
		"	tst	%0, #0\n\t"
		"	beq	1f\n\t"
		"	str	%1, [%0, #" __stringify(AT91_DDRSDRC_LPR) "]\n\t"
		/* Shutdown CPU */
		"1:	str	%3, [%2, #" __stringify(AT91_SHDW_CR) "]\n\t"

		"	b	.\n\t"
		:
		: "r" (at91_shdwc->mpddrc_base),
		  "r" cpu_to_le32(AT91_DDRSDRC_LPDDR2_PWOFF),
		  "r" (at91_shdwc->shdwc_base),
		  "r" cpu_to_le32(AT91_SHDW_KEY | AT91_SHDW_SHDW)
		: "r6");

	gpiod_set_value(pio_out, 0);

	return IRQ_HANDLED;
}

static int aic5_irq_pin_probe(struct platform_device *pdev)
{
	struct resource   *regs;
	int     irq;
	int     ret;

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs)
		return -ENXIO;

	aic5_base = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(aic5_base)) {
	    return PTR_ERR(aic5_base);
	}
	printk(KERN_ERR "%s done 0\n", __func__);

	pio_out = devm_gpiod_get(&pdev->dev, "out", GPIOD_OUT_LOW);
	if (IS_ERR(pio_out))
           return PTR_ERR(pio_out);

	printk(KERN_ERR "%s done 1\n", __func__);
	// external IRQ periphral ID is 49
	irq = platform_get_irq(pdev, 0);
	printk(KERN_ERR "%s irq %d\n", __func__, irq);
	if (irq < 0)
		return irq;

	printk(KERN_ERR "%s done 2\n", __func__);
	local_irq_disable();

	ret = devm_request_irq(&pdev->dev, irq, irq_pin_interrupt,
					0, dev_name(&pdev->dev), NULL);
	if (ret)
		goto out_free_irq;

	local_irq_enable();

	printk(KERN_ERR "%s done.\n", __func__);
	//dev_dbg("%s done.\n", __func__);
	//dev_dbg(&pdev->dev, "%s done.\n", __func__);

	return 0;
out_free_irq:
	local_irq_enable();
	return ret;
}

static int aic5_irq_pin_remove(struct platform_device *pdev)
{
	/* Nothing to do */
	return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id aic5_irq_pin_dt_ids[] = {
	{ .compatible = "atmel,sama5d2-irq-pin" },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, aic5_irq_pin_dt_ids);
#endif

static struct platform_driver aic5_irq_pin_driver = {
  .driver   = {
    .name   = "aic5-irq-pin",
    .of_match_table = of_match_ptr(aic5_irq_pin_dt_ids),
  },
  .probe    = aic5_irq_pin_probe,
  .remove   = aic5_irq_pin_remove,
};
module_platform_driver(aic5_irq_pin_driver);

MODULE_AUTHOR("Matt Wood <matt.wood@microchip.com>");
MODULE_DESCRIPTION("SAMA5D2 irq pin driver");
MODULE_LICENSE("GPL");

