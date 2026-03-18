/* =========================================================================
 * The Synopsys DWC ETHER QOS Software Driver and documentation (hereinafter
 * "Software") is an unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto.  Permission is hereby granted,
 * free of charge, to any person obtaining a copy of this software annotated
 * with this license and the Software, to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * ========================================================================= */

/*!@file: dwc_eth_qos.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"
#if IS_ENABLED(CONFIG_PCI)
#include <linux/soc/nvt/nvt-pcie-lib.h>
#endif

#ifdef CONFIG_OF
#include <linux/of_device.h>
#endif

#ifdef CONFIG_PM
static INT DWC_ETH_QOS_resume(struct platform_device *pdev);
static INT DWC_ETH_QOS_suspend(struct platform_device *pdev, pm_message_t state);
#endif

void DWC_ETH_QOS_init_all_fptrs(struct DWC_ETH_QOS_prv_data *pdata)
{
	DWC_ETH_QOS_init_function_ptrs_dev(&pdata->hw_if);
	DWC_ETH_QOS_init_function_ptrs_desc(&pdata->desc_if);
}

#if IS_ENABLED(CONFIG_PCI)
nvtpcie_chipid_t nvt_eth_dts_chipid(struct device_node *np)
{
	u32 u32prop;
	nvtpcie_chipid_t chipid = CHIP_RC;

#ifdef CONFIG_OF
	if (0 == of_property_read_u32(np, "chip_id", &u32prop)) {
		chipid = (nvtpcie_chipid_t)u32prop;
	}
#endif
	return chipid;
}

int nvt_eth_ep_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, struct device_node *np)
{
	u32 u32prop;
	int ret = 0;

#ifdef CONFIG_OF
	if (0 == of_property_read_u32(np, "pinmux", &u32prop)) {
		pinmux_config[0].config = u32prop;
	}
#endif
	if (!pinmux_config[0].config)
		ret = -1;

	return ret;
}

#endif

/*!
* \brief API to initialize the device.
*
* \details This probing function gets called (during execution of
* pci_register_driver() for already existing devices or later if a
* new device gets inserted) for all PCI devices which match the ID table
* and are not "owned" by the other drivers yet. This function gets passed
* a "struct pci_dev *" for each device whose entry in the ID table matches
* the device. The probe function returns zero when the driver chooses to take
* "ownership" of the device or an error code (negative number) otherwise.
* The probe function always gets called from process context, so it can sleep.
*
* \param[in] pdev - pointer to platform_device structure.
*
* \return integer
*
* \retval 0 on success & -ve number on failure.
*/

static int DWC_ETH_QOS_probe(struct platform_device *pdev)
{

	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct net_device *dev = NULL;
	int i, ret = 0;
	int idx_mac;
	struct hw_if_struct *hw_if = NULL;
	struct desc_if_struct *desc_if = NULL;
	UCHAR tx_q_count = 0, rx_q_count = 0;
	struct clk *clk;
	void __iomem *base;
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NA51055)
	int embd_en = 0;
#endif

	DBGPR("--> DWC_ETH_QOS_probe\n");
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NA51055)
	idx_mac = nvt_eth_env_probe(pdev, &base, &embd_en);
#else
	idx_mac = nvt_eth_env_probe(pdev, &base);
#endif
	if (idx_mac < 0) {
		return -ENXIO;
	}

#if IS_ENABLED(CONFIG_MULTI_QUEUE)
	/* queue count */
	tx_q_count = probe_get_tx_queue_count((unsigned long)base);
	rx_q_count = probe_get_rx_queue_count((unsigned long)base);
#else
	tx_q_count = 1;
	rx_q_count = 1;
#endif

	dev = alloc_etherdev_mqs(sizeof(struct DWC_ETH_QOS_prv_data),
							 tx_q_count, rx_q_count);
	if (dev == NULL) {
		printk(KERN_ALERT "%s:Unable to alloc new net device\n",
			   DEV_NAME);
		ret = -ENOMEM;
		goto err_out_dev_failed;
	}

	if (!is_valid_ether_addr(dev->dev_addr)) {
		dev_info(&pdev->dev, "Assigning random MAC address.\n");
		eth_hw_addr_random(dev);
	}

	dev->min_mtu = DWC_ETH_QOS_MIN_SUPPORTED_MTU;
	dev->max_mtu = DWC_ETH_QOS_MAX_GPSL;
	dev->base_addr = (unsigned long)base;
	DBGPR("--> base_addr = %llx\n", dev->base_addr);
	SET_NETDEV_DEV(dev, &pdev->dev);
	pdata = netdev_priv(dev);
	DWC_ETH_QOS_init_all_fptrs(pdata);
	hw_if = &(pdata->hw_if);
	desc_if = &(pdata->desc_if);
	platform_set_drvdata(pdev, dev);
	pdata->pdev = pdev;
#ifdef CONFIG_MDIO_INT_MODE
	mutex_init(&pdata->mdio_idle);
	init_completion(&pdata->mdio_intr);
#endif
	pdata->dev = dev;
	pdata->tx_queue_cnt = tx_q_count;
	pdata->rx_queue_cnt = rx_q_count;

	pdata->bus_id = idx_mac;
#if IS_ENABLED(CONFIG_PCI)
	pdata->ep_id = nvt_eth_dts_chipid(pdev->dev.of_node);
#else
	pdata->ep_id = CHIP_RC;
#endif
#ifdef CONFIG_PM
	nvt_eth_get_clk_name(pdata, idx_mac);
#endif

#ifdef CONFIG_OF
	ret = of_property_read_u32(pdev->dev.of_node, "sp-clk", &pdata->spclk_en);
	if (ret)
		dev_err(&pdev->dev, "can't find sp-clk %d\n", pdata->spclk_en);

	if (pdata->ep_id == CHIP_RC) {
		dev_info(&pdev->dev, "%s, clk_name = %s\n", __func__, dev_name(&pdev->dev));
		clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
			clk = NULL;
		} else {
			clk_prepare(clk);
			clk_enable(clk);
		}
		clk_put(clk);
	}
//	nvt_disable_sram_shutdown(ETH_SD);   // temp skip SRAM control
#endif



#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	create_debug_files(pdata);
	/* to give prv data to debugfs */
//	DWC_ETH_QOS_get_pdata(pdata);
#endif
	/* issue software reset to device */
	hw_if->exit(pdata);


	DWC_ETH_QOS_get_all_hw_features(pdata);
	DWC_ETH_QOS_print_all_hw_features(pdata);
	pdata->max_addr_reg_cnt = 32;

	ret = desc_if->alloc_queue_struct(pdata);
	if (ret < 0) {
		printk(KERN_ALERT "ERROR: Unable to alloc Tx/Rx queue\n");
		goto err_out_q_alloc_failed;
	}

#if IS_ENABLED(CONFIG_MULTI_IRQ)
	dev->irq = platform_get_irq(pdev, 0);
	for (i = 1; i <= DWC_ETH_QOS_TX_QUEUE_CNT; i++) {
		pdata->tx_queue[i - 1].tx_irq = platform_get_irq(pdev, i);
	}
	for (i = DWC_ETH_QOS_TX_QUEUE_CNT + 1; i <= DWC_ETH_QOS_RX_QUEUE_CNT + DWC_ETH_QOS_TX_QUEUE_CNT; i++) {
		pdata->rx_queue[i - 5].rx_irq = platform_get_irq(pdev, i);
	}
#else
	dev->irq = platform_get_irq(pdev, 0);
#endif

	dev->netdev_ops = DWC_ETH_QOS_get_netdev_ops();

	pdata->interface = DWC_ETH_QOS_get_phy_interface(pdata);

#if CONFIG_OF
	if (of_phy_is_fixed_link(pdev->dev.of_node)) {
		printk("%s: fixed link found\r\n", __func__);
		if (of_phy_register_fixed_link(pdev->dev.of_node) == 0) {
			pdata->phy_node = of_node_get(pdev->dev.of_node);
			printk("%s: register fixed phy ok\r\n", __func__);
		}
	}
#endif
	/* Bypass PHYLIB for TBI, RTBI and SGMII interface */
	if (1 == pdata->hw_feat.sma_sel) {
		ret = DWC_ETH_QOS_mdio_register(dev, pdata->ep_id);
		if (ret < 0) {
			printk(KERN_ALERT "MDIO bus (id %d) registration failed\n",
				   pdata->bus_id);
			goto err_out_mdio_reg;
		}
	} else {
		printk(KERN_ALERT "%s: MDIO is not present\n\n", DEV_NAME);
#ifndef _NVT_FPGA_
		printk(KERN_ALERT "Plz ensure you select correct FPGA bit file\n");
#endif
	}
#ifdef TIMER_CONTROL
	printk(KERN_ALERT "%s: Enable TIMTER CONTROL value %d\n", DEV_NAME, TIMER_LATENCY);
#endif
#ifndef DWC_ETH_QOS_CONFIG_PGTEST
	/* enabling and registration of irq with magic wakeup */
	if (1 == pdata->hw_feat.mgk_sel) {
		device_set_wakeup_capable(&pdev->dev, 1);
		// coverity[missing_lock], probe here is no race condition.
		pdata->wolopts = WAKE_MAGIC;
		enable_irq_wake(dev->irq);
	}
#if IS_ENABLED(CONFIG_MULTI_IRQ)

	for (i = 0; i < DWC_ETH_QOS_RX_QUEUE_CNT; i++) {
		struct DWC_ETH_QOS_rx_queue *rx_queue = GET_RX_QUEUE_PTR(i);
		struct DWC_ETH_QOS_tx_queue *tx_queue = GET_TX_QUEUE_PTR(i);

		netif_napi_add(dev, &rx_queue->napi, DWC_ETH_QOS_rx_poll_mq,
					   (64 * DWC_ETH_QOS_RX_QUEUE_CNT));

		netif_napi_add(dev, &tx_queue->napi, DWC_ETH_QOS_tx_poll_mq,
					   (64 * DWC_ETH_QOS_TX_QUEUE_CNT));
	}
#else
	for (i = 0; i < DWC_ETH_QOS_RX_QUEUE_CNT; i++) {
		struct DWC_ETH_QOS_rx_queue *rx_queue = GET_RX_QUEUE_PTR(i);

		netif_napi_add(dev, &rx_queue->napi, DWC_ETH_QOS_poll_mq,
					   (64 * DWC_ETH_QOS_RX_QUEUE_CNT));
	}
#endif

	dev->ethtool_ops = DWC_ETH_QOS_get_ethtool_ops();
	dev->hw_features = NETIF_F_SG;
	printk(KERN_ALERT "Supports SG\n");

	if (pdata->hw_feat.tso_en) {
		dev->hw_features |= NETIF_F_TSO;
		printk(KERN_ALERT "Supports TSO\n");
	}

	if (pdata->hw_feat.tx_coe_sel) {
		dev->hw_features |= NETIF_F_IP_CSUM ;
		dev->hw_features |= NETIF_F_IPV6_CSUM;
		printk(KERN_ALERT "Supports TX COE\n");
	}

	if (pdata->hw_feat.rx_coe_sel) {
		dev->hw_features |= NETIF_F_RXCSUM;
		dev->hw_features |= NETIF_F_GRO;
		printk(KERN_ALERT "Supports RX COE and GRO\n");
	}
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	dev->vlan_features |= dev->hw_features;
	dev->hw_features |= NETIF_F_HW_VLAN_CTAG_RX;
	if (pdata->hw_feat.sa_vlan_ins) {
		dev->hw_features |= NETIF_F_HW_VLAN_CTAG_TX;
		printk(KERN_ALERT "VLAN Feature enabled\n");
	}
	if (pdata->hw_feat.vlan_hash_en) {
		dev->hw_features |= NETIF_F_HW_VLAN_CTAG_FILTER;
		printk(KERN_ALERT "VLAN HASH Filtering enabled\n");
	}
#endif /* end of DWC_ETH_QOS_ENABLE_VLAN_TAG */
	dev->features |= dev->hw_features;
	pdata->dev_state |= dev->features;

	DWC_ETH_QOS_init_rx_coalesce(pdata);
#ifdef DWC_ETH_QOS_ENABLE_IRQ_MOD
	pdata->tx_irq_mod_lim = NVT_ETH_TX_IRQ_MOD_TMR_LIM;
	pdata->rx_irq_mod_lim = NVT_ETH_RX_IRQ_MOD_TMR_LIM;
#endif

#if IS_ENABLED(CONFIG_TIMESTAMP)
	DWC_ETH_QOS_ptp_init(pdata);
#endif  /* end of DWC_ETH_QOS_CONFIG_PTP */

#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

	// coverity[side_effect_free]
	spin_lock_init(&pdata->lock);
	// coverity[side_effect_free]
	spin_lock_init(&pdata->tx_lock);
	// coverity[side_effect_free]
	spin_lock_init(&pdata->pmt_lock);
	// coverity[side_effect_free]
	spin_lock_init(&pdata->rx_int_lock);
	// coverity[side_effect_free]
	spin_lock_init(&pdata->rx_int_lock);
#if IS_ENABLED(CONFIG_MULTI_IRQ)
	for (i = 0; i < DWC_ETH_QOS_RX_QUEUE_CNT; i++) {
		struct DWC_ETH_QOS_rx_queue *rx_queue = GET_RX_QUEUE_PTR(i);
		spin_lock_init(&rx_queue->rx_dma_lock);
	}
	for (i = 0; i < DWC_ETH_QOS_TX_QUEUE_CNT; i++) {
		struct DWC_ETH_QOS_tx_queue *tx_queue = GET_TX_QUEUE_PTR(i);
		spin_lock_init(&tx_queue->tx_dma_lock);
	}
#endif
	mutex_init(&pdata->ethtool_lock);

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	ret = DWC_ETH_QOS_alloc_pg(pdata);
	if (ret < 0) {
		printk(KERN_ALERT "ERROR:Unable to allocate PG memory\n");
		goto err_out_pg_failed;
	}
	printk(KERN_ALERT "\n");
	printk(KERN_ALERT "/*******************************************\n");
	printk(KERN_ALERT "*\n");
	printk(KERN_ALERT "* PACKET GENERATOR MODULE ENABLED IN DRIVER\n");
	printk(KERN_ALERT "*\n");
	printk(KERN_ALERT "*******************************************/\n");
	printk(KERN_ALERT "\n");
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

	ret = register_netdev(dev);
	if (ret) {
		printk(KERN_ALERT "%s: Net device registration failed\n",
			   DEV_NAME);
		goto err_out_reg_netdev_failed;
	}

#if defined(CONFIG_NVT_IVOT_PLAT_NA51103) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NA51055)
	pdata->embd_phy = embd_en;
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
	nvt_eth_proc_init(pdata, idx_mac);
#endif
#endif

	DBGPR("<-- DWC_ETH_QOS_probe\n");
	if (!dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(DWC_ETH_QOS_DMA_MASK))) {
		printk("set mask to %dbit\n", DWC_ETH_QOS_DMA_MASK);
	} else {
		printk("set mask to %dbit fail!\n", DWC_ETH_QOS_DMA_MASK);
	}

	if (pdata->hw_feat.pcs_sel) {
		netif_carrier_off(dev);
		printk(KERN_ALERT "carrier off till LINK is up\n");
	}

	return 0;

err_out_dev_failed:
#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_remove(pdata);
#endif  /* end of DWC_ETH_QOS_CONFIG_PTP */
	platform_set_drvdata(pdev, NULL);
	return ret;

err_out_reg_netdev_failed:
#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	DWC_ETH_QOS_free_pg(pdata);
err_out_pg_failed:
#endif
	if (1 == pdata->hw_feat.sma_sel) {
		DWC_ETH_QOS_mdio_unregister(dev);
	}

err_out_mdio_reg:
	desc_if->free_queue_struct(pdata);

err_out_q_alloc_failed:
#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	remove_debug_files(pdata);
#endif
	free_netdev(dev);
	return ret;
}

/*!
* \brief API to release all the resources from the driver.
*
* \details The remove function gets called whenever a device being handled
* by this driver is removed (either during deregistration of the driver or
* when it is manually pulled out of a hot-pluggable slot). This function
* should reverse operations performed at probe time. The remove function
* always gets called from process context, so it can sleep.
*
* \param[in] pdev - pointer to platform_device structure.
*
* \return void
*/
static int DWC_ETH_QOS_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
//	struct desc_if_struct *desc_if = &(pdata->desc_if);
	struct clk *clk;

#if IS_ENABLED(CONFIG_MULTI_IRQ)
	int i;
#endif

	DBGPR("--> DWC_ETH_QOS_remove\n");

#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	remove_debug_files(pdata);
#endif

	if (pdata->irq_number != 0) {
		free_irq(pdata->irq_number, pdata);
		pdata->irq_number = 0;
	}

#if IS_ENABLED(CONFIG_MULTI_IRQ)
	for (i = 0; i < DWC_ETH_QOS_RX_QUEUE_CNT; i++) {
		struct DWC_ETH_QOS_rx_queue *rx_queue = GET_RX_QUEUE_PTR(i);
		if (rx_queue->rx_irq_flag != 0) {
			free_irq(rx_queue->rx_irq, rx_queue);
			rx_queue->rx_irq_flag = 0;
		}
	}
	for (i = 0; i < DWC_ETH_QOS_TX_QUEUE_CNT; i++) {
		struct DWC_ETH_QOS_tx_queue *tx_queue = GET_TX_QUEUE_PTR(i);
		if (tx_queue->tx_irq_flag != 0) {
			free_irq(tx_queue->tx_irq, tx_queue);
			tx_queue->tx_irq_flag = 0;
		}
	}
#endif

#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_remove(pdata);
#endif /* end of DWC_ETH_QOS_CONFIG_PTP */

	netif_carrier_off(dev);
	unregister_netdev(dev);

	if (1 == pdata->hw_feat.sma_sel) {
		DWC_ETH_QOS_mdio_unregister(dev);
	}

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	DWC_ETH_QOS_free_pg(pdata);
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

	// later free_netdev() will access pointer in allocated queue.
	// If we free queue buffer here, later will encounter kernel exception.
	// ***BUT** queue buffer is allocated by devm_alloc(), so just skip free
//	desc_if->free_queue_struct(pdata);

#ifdef CONFIG_OF
	if (pdata->phy_node) {
		of_node_put(pdata->phy_node);
	}

	clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
		clk = NULL;
	} else {
		clk_disable(clk);
		clk_unprepare(clk);
	}
	clk_put(clk);
#endif

#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
	nvt_eth_proc_remove(pdata);
#endif

	free_netdev(dev);

	DBGPR("<-- DWC_ETH_QOS_remove\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id synopsys_eth_of_dt_ids[] = {
	{ .compatible = "nvt,synopsys_eth" },
	{},
};
MODULE_DEVICE_TABLE(of, synopsys_eth_of_dt_ids);
#endif

static struct platform_driver DWC_ETH_QOS_driver = {
	.probe = DWC_ETH_QOS_probe,
	.remove = DWC_ETH_QOS_remove,
#ifdef CONFIG_PM
	.suspend = DWC_ETH_QOS_suspend,
	.resume = DWC_ETH_QOS_resume,
#endif
	.driver = {
		.name = DEV_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = synopsys_eth_of_dt_ids,
#endif
	},
};

#ifdef CONFIG_PM

/*!
 * \brief Routine to put the device in suspend mode
 *
 * \details This function gets called by PCI core when the device is being
 * suspended. The suspended state is passed as input argument to it.
 * Following operations are performed in this function,
 * - stop the phy.
 * - detach the device from stack.
 * - stop the queue.
 * - Disable napi.
 * - Stop DMA TX and RX process.
 * - Enable power down mode using PMT module or disable MAC TX and RX process.
 * - Save the pci state.
 *
 * \param[in] pdev ??pointer to platform_device structure.
 * \param[in] state ??suspend state of device.
 *
 * \return int
 *
 * \retval 0
 */

static INT DWC_ETH_QOS_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct clk *clk;
	INT ret = 0;

	DBGPR("-->DWC_ETH_QOS_suspend\n");

	if (!dev || !netif_running(dev)) {
		printk("@@ <--DWC_ETH_QOS_dev_suspend\n");
	} else {
		DWC_ETH_QOS_drv_suspend(dev);
	}

#ifdef CONFIG_OF
	clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
		clk = NULL;
	} else {
		clk_disable(clk);
		clk_unprepare(clk);
	}
	clk_put(clk);

	if (pdata->spclk_en && !pdata->ep_id) {

		clk = clk_get(&pdev->dev, pdata->ext_phy_clk_name);
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "can't find ext_phy_clock %s\n",
					pdata->ext_phy_clk_name);
			clk = NULL;
		} else {
			clk_disable(clk);
			clk_unprepare(clk);
		}
		clk_put(clk);
	}
#if IS_ENABLED(CONFIG_TIMESTAMP)
	if (pdata->ep_id == 0) {
		clk = clk_get(&pdev->dev, pdata->ptp_clk_name);
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "can't find ptp_clock %s\n",
					pdata->ptp_clk_name);
			clk = NULL;
		} else {
			clk_disable(clk);
			clk_unprepare(clk);
		}
		clk_put(clk);
	}
#endif
#endif

	DBGPR("<--DWC_ETH_QOS_suspend\n");

	return ret;
}

/*!
 * \brief Routine to resume device operation
 *
 * \details This function gets called by PCI core when the device is being
 * resumed. It is always called after suspend has been called. These function
 * reverse operations performed at suspend time. Following operations are
 * performed in this function,
 * - restores the saved pci power state.
 * - Wakeup the device using PMT module if supported.
 * - Starts the phy.
 * - Enable MAC and DMA TX and RX process.
 * - Attach the device to stack.
 * - Enable napi.
 * - Starts the queue.
 *
 * \param[in] pdev ?�pointer to platform_device structure.
 *
 * \return int
 *
 * \retval 0
 */

static INT DWC_ETH_QOS_resume(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct clk *clk;
	INT ret = 0;

	DBGPR("-->DWC_ETH_QOS_resume\n");

#ifdef CONFIG_OF
	clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "can't find clock %s\n", dev_name(&pdev->dev));
		clk = NULL;
	} else {
		clk_prepare(clk);
		clk_enable(clk);
	}
	clk_put(clk);

	if (pdata->spclk_en && !pdata->ep_id) {

		clk = clk_get(&pdev->dev, pdata->ext_phy_clk_name);
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "can't find ext_phy_clock %s\n",
					pdata->ext_phy_clk_name);
			clk = NULL;
		} else {
			clk_prepare(clk);
			clk_enable(clk);
		}
		clk_put(clk);
	}
#if IS_ENABLED(CONFIG_TIMESTAMP)
	if (pdata->ep_id == 0) {
		clk = clk_get(&pdev->dev, pdata->ptp_clk_name);
		if (IS_ERR(clk)) {
			dev_err(&pdev->dev, "can't find ptp_clock %s\n",
					pdata->ptp_clk_name);
			clk = NULL;
		} else {
			clk_prepare(clk);
			clk_enable(clk);
		}
		clk_put(clk);
	}
#endif
#endif

	if (!dev || !netif_running(dev)) {
		printk("@@ <--DWC_ETH_QOS_dev_resume\n");
		return 0;
	}

	DWC_ETH_QOS_drv_resume(dev);

	DBGPR("<--DWC_ETH_QOS_resume\n");

	return ret;
}

#endif  /* CONFIG_PM */

/*!
* \brief Macro to register the driver registration function.
*
* \details A module always begin with either the init_module or the function
* you specify with module_init call. This is the entry function for modules;
* it tells the kernel what functionality the module provides and sets up the
* kernel to run the module's functions when they're needed. Once it does this,
* entry function returns and the module does nothing until the kernel wants
* to do something with the code that the module provides.
*/
module_platform_driver(DWC_ETH_QOS_driver);


/*!
* \brief Macro to declare the module author.
*
* \details This macro is used to declare the module's authore.
*/
MODULE_AUTHOR("Synopsys India Pvt Ltd");

/*!
* \brief Macro to describe what the module does.
*
* \details This macro is used to describe what the module does.
*/
MODULE_DESCRIPTION("DWC_ETH_QOS Driver");

/*!
* \brief Macro to describe the module license.
*
* \details This macro is used to describe the module license.
*/
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL");
