/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */


#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#include "lens_info.h"
#include "lens_list.h"

#define AF_DRVNAME "MAINAF"
#if defined(CONFIG_MTK_LEGACY)
#define I2C_CONFIG_SETTING 1
#elif defined(CONFIG_OF)
#define I2C_CONFIG_SETTING 2 
#else

#define I2C_CONFIG_SETTING 1
#endif

#if I2C_CONFIG_SETTING == 1
#define LENS_I2C_BUSNUM 2
#define I2C_REGISTER_ID            0x72
#endif


#define PLATFORM_DRIVER_NAME "lens_actuator_main_af"
#define AF_DRIVER_CLASS_NAME "actuatordrv_main_af"


#if I2C_CONFIG_SETTING == 1
static struct i2c_board_info kd_lens_dev __initdata = {
	I2C_BOARD_INFO(AF_DRVNAME, I2C_REGISTER_ID)
};
#endif

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...) pr_debug(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif


static stAF_DrvList g_stAF_DrvList[MAX_NUM_OF_LENS] = {
	#ifdef CONFIG_MTK_LENS_BU6424AF_SUPPORT
	{1, AFDRV_BU6424AF, BU6424AF_SetI2Cclient, BU6424AF_Ioctl, BU6424AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_BU6429AF_SUPPORT
	{1, AFDRV_BU6429AF, BU6429AF_SetI2Cclient, BU6429AF_Ioctl, BU6429AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_BU63165AF_SUPPORT
	{1, AFDRV_BU63165AF, BU63165AF_SetI2Cclient, BU63165AF_Ioctl, BU63165AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_DW9714AF_SUPPORT
	{1, AFDRV_DW9714AF, DW9714AF_SetI2Cclient, DW9714AF_Ioctl, DW9714AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_DW9814AF_SUPPORT
	{1, AFDRV_DW9814AF, DW9814AF_SetI2Cclient, DW9814AF_Ioctl, DW9814AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_DW9718AF_SUPPORT
	{1, AFDRV_DW9718AF, DW9718AF_SetI2Cclient, DW9718AF_Ioctl, DW9718AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_LC898122AF_SUPPORT
	{1, AFDRV_LC898122AF, LC898122AF_SetI2Cclient, LC898122AF_Ioctl, LC898122AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_LC898212AF_SUPPORT
	{1, AFDRV_LC898212AF, LC898212AF_SetI2Cclient, LC898212AF_Ioctl, LC898212AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_LC898212XDAF_SUPPORT
	{1, AFDRV_LC898212XDAF, LC898212XDAF_F_SetI2Cclient, LC898212XDAF_F_Ioctl, LC898212XDAF_F_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_LC898214AF_SUPPORT
	{1, AFDRV_LC898214AF, LC898214AF_SetI2Cclient, LC898214AF_Ioctl, LC898214AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_FM50AF_SUPPORT
	{1, AFDRV_FM50AF, FM50AF_SetI2Cclient, FM50AF_Ioctl, FM50AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_AD5820AF_SUPPORT
	{1, AFDRV_AD5820AF, AD5820AF_SetI2Cclient, AD5820AF_Ioctl, AD5820AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_WV511AAF_SUPPORT
	{1, AFDRV_WV511AAF, WV511AAF_SetI2Cclient, WV511AAF_Ioctl, WV511AAF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_AK7371AF_SUPPORT
	{1, AFDRV_AK7371AF, AK7371AF_SetI2Cclient, AK7371AF_Ioctl, AK7371AF_Release},
	#endif
	#ifdef CONFIG_MTK_LENS_LC898123AF_OPENLOOP_SUPPORT
	{1, AFDRV_LC898123AF_OPENLOOP, LC898123AF_SetI2Cclient, LC898123AF_Ioctl, LC898123AF_Release},
	#endif
};

static stAF_DrvList *g_pstAF_CurDrv;

static spinlock_t g_AF_SpinLock;

static int g_s4AF_Opened;

static struct i2c_client *g_pstAF_I2Cclient;

static dev_t g_AF_devno;
static struct cdev *g_pAF_CharDrv;
static struct class *actuator_class;

static long AF_SetMotorName(__user stAF_MotorName * pstMotorName)
{
	long i4RetValue = -1;
	int i;
	stAF_MotorName stMotorName;

	if (copy_from_user(&stMotorName , pstMotorName, sizeof(stAF_MotorName)))
		LOG_INF("copy to user failed when getting motor information\n");

	LOG_INF("Set Motor Name : %s\n", stMotorName.uMotorName);

	for (i = 0; i < MAX_NUM_OF_LENS; i++) {
		if (g_stAF_DrvList[i].uEnable != 1)
			break;

		LOG_INF("Search Motor Name : %s\n", g_stAF_DrvList[i].uDrvName);
		if (strcmp(stMotorName.uMotorName, g_stAF_DrvList[i].uDrvName) == 0) {
			g_pstAF_CurDrv = &g_stAF_DrvList[i];
			g_pstAF_CurDrv->pAF_SetI2Cclient(g_pstAF_I2Cclient, &g_AF_SpinLock, &g_s4AF_Opened);
			i4RetValue = 1;
			break;
		}
	}
	return i4RetValue;
}

static long AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command, unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_S_SETDRVNAME:
		i4RetValue = AF_SetMotorName((__user stAF_MotorName *)(a_u4Param));
		break;

	default:
		if (g_pstAF_CurDrv)
			i4RetValue = g_pstAF_CurDrv->pAF_Ioctl(a_pstFile, a_u4Command, a_u4Param);
		break;
	}

	return i4RetValue;
}

#ifdef CONFIG_COMPAT
static long AF_Ioctl_Compat(struct file *a_pstFile, unsigned int a_u4Command, unsigned long a_u4Param)
{
	long i4RetValue = 0;

	i4RetValue = AF_Ioctl(a_pstFile, a_u4Command, (unsigned long)compat_ptr(a_u4Param));

	return i4RetValue;
}
#endif

static int AF_Open(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (g_s4AF_Opened) {
		LOG_INF("The device is opened\n");
		return -EBUSY;
	}

	spin_lock(&g_AF_SpinLock);
	g_s4AF_Opened = 1;
	spin_unlock(&g_AF_SpinLock);

	LOG_INF("End\n");

	return 0;
}

static int AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (g_pstAF_CurDrv) {
		g_pstAF_CurDrv->pAF_Release(a_pstInode, a_pstFile);
		g_pstAF_CurDrv = NULL;
	} else {
		spin_lock(&g_AF_SpinLock);
		g_s4AF_Opened = 0;
		spin_unlock(&g_AF_SpinLock);
	}

	LOG_INF("End\n");

	return 0;
}

static const struct file_operations g_stAF_fops = {
	.owner = THIS_MODULE,
	.open = AF_Open,
	.release = AF_Release,
	.unlocked_ioctl = AF_Ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = AF_Ioctl_Compat,
#endif
};

static inline int Register_AF_CharDrv(void)
{
	struct device *vcm_device = NULL;

	LOG_INF("Start\n");

	
	if (alloc_chrdev_region(&g_AF_devno, 0, 1, AF_DRVNAME)) {
		LOG_INF("Allocate device no failed\n");

		return -EAGAIN;
	}
	
	g_pAF_CharDrv = cdev_alloc();

	if (NULL == g_pAF_CharDrv) {
		unregister_chrdev_region(g_AF_devno, 1);

		LOG_INF("Allocate mem for kobject failed\n");

		return -ENOMEM;
	}
	
	cdev_init(g_pAF_CharDrv, &g_stAF_fops);

	g_pAF_CharDrv->owner = THIS_MODULE;

	
	if (cdev_add(g_pAF_CharDrv, g_AF_devno, 1)) {
		LOG_INF("Attatch file operation failed\n");

		unregister_chrdev_region(g_AF_devno, 1);

		return -EAGAIN;
	}

	actuator_class = class_create(THIS_MODULE, AF_DRIVER_CLASS_NAME);
	if (IS_ERR(actuator_class)) {
		int ret = PTR_ERR(actuator_class);

		LOG_INF("Unable to create class, err = %d\n", ret);
		return ret;
	}

	vcm_device = device_create(actuator_class, NULL, g_AF_devno, NULL, AF_DRVNAME);

	if (NULL == vcm_device)
		return -EIO;

	LOG_INF("End\n");
	return 0;
}

static inline void Unregister_AF_CharDrv(void)
{
	LOG_INF("Start\n");

	
	cdev_del(g_pAF_CharDrv);

	unregister_chrdev_region(g_AF_devno, 1);

	device_destroy(actuator_class, g_AF_devno);

	class_destroy(actuator_class);

	LOG_INF("End\n");
}


static int AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id AF_i2c_id[] = { {AF_DRVNAME, 0}, {} };

#if I2C_CONFIG_SETTING == 2
static const struct of_device_id MAINAF_of_match[] = {
	{.compatible = "mediatek,CAMERA_MAIN_AF"},
	{},
};
#endif

static struct i2c_driver AF_i2c_driver = {
	.probe = AF_i2c_probe,
	.remove = AF_i2c_remove,
	.driver.name = AF_DRVNAME,
#if I2C_CONFIG_SETTING == 2
	.driver.of_match_table = MAINAF_of_match,
#endif
	.id_table = AF_i2c_id,
};

static int AF_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static int AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int i4RetValue = 0;

	LOG_INF("Start\n");

	
	g_pstAF_I2Cclient = client;

	
	i4RetValue = Register_AF_CharDrv();

	if (i4RetValue) {

		LOG_INF(" register char device failed!\n");

		return i4RetValue;
	}

	spin_lock_init(&g_AF_SpinLock);

       
#ifdef ALPINE_CAMERA
	LC898123AF_SetI2Cclient(g_pstAF_I2Cclient, &g_AF_SpinLock, &g_s4AF_Opened);
#endif

	LOG_INF("Attached!!\n");

	return 0;
}

static int AF_probe(struct platform_device *pdev)
{
	return i2c_add_driver(&AF_i2c_driver);
}

static int AF_remove(struct platform_device *pdev)
{
	i2c_del_driver(&AF_i2c_driver);
	return 0;
}

static int AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	return 0;
}

static int AF_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver g_stAF_Driver = {
	.probe = AF_probe,
	.remove = AF_remove,
	.suspend = AF_suspend,
	.resume = AF_resume,
	.driver = {
		   .name = PLATFORM_DRIVER_NAME,
		   .owner = THIS_MODULE,
		   }
};

static struct platform_device g_stAF_device = {
	.name = PLATFORM_DRIVER_NAME,
	.id = 0,
	.dev = {}
};

static int __init MAINAF_i2C_init(void)
{
	#if I2C_CONFIG_SETTING == 1
	i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
	#endif

	if (platform_device_register(&g_stAF_device)) {
		LOG_INF("failed to register AF driver\n");
		return -ENODEV;
	}

	if (platform_driver_register(&g_stAF_Driver)) {
		LOG_INF("Failed to register AF driver\n");
		return -ENODEV;
	}

	return 0;
}

static void __exit MAINAF_i2C_exit(void)
{
	platform_driver_unregister(&g_stAF_Driver);
}
module_init(MAINAF_i2C_init);
module_exit(MAINAF_i2C_exit);

MODULE_DESCRIPTION("MAINAF lens module driver");
MODULE_AUTHOR("KY Chen <ky.chen@Mediatek.com>");
MODULE_LICENSE("GPL");