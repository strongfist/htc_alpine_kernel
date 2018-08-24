/*
 *
 * FocalTech TouchScreen driver.
 * 
 * Copyright (c) 2010-2015, Focaltech Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include "focaltech_core.h"

#define PROC_UPGRADE							0
#define PROC_READ_REGISTER						1
#define PROC_WRITE_REGISTER						2
#define PROC_AUTOCLB							4
#define PROC_UPGRADE_INFO						5
#define PROC_WRITE_DATA							6
#define PROC_READ_DATA							7
#define PROC_SET_TEST_FLAG						8
#define FTS_DEBUG_DIR_NAME						"fts_debug"
#define PROC_NAME								"ft5x0x-debug"
#define WRITE_BUF_SIZE							1016
#define READ_BUF_SIZE							1016



static unsigned char proc_operate_mode 			= PROC_UPGRADE;
static struct proc_dir_entry *fts_proc_entry;
extern int apk_debug_flag;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 10, 0))
static ssize_t fts_debug_write(struct file *filp, const char __user *buff, size_t count, loff_t *ppos)
{
	unsigned char writebuf[WRITE_BUF_SIZE];
	int buflen = count;
	int writelen = 0;
	int ret = 0;
	printk("fts rawdata fts_debug_write\n");
	#if FT_ESD_PROTECT
    
    esd_switch(0);apk_debug_flag = 1;
   

	#endif
	if (copy_from_user(&writebuf, buff, buflen)) {
		dev_err(&fts_i2c_client->dev, "%s:copy from user error\n", __func__);
    #if FT_ESD_PROTECT
	esd_switch(1);apk_debug_flag = 0;
	#endif
		return -EFAULT;
	}
	proc_operate_mode = writebuf[0];

	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		{
			char upgrade_file_path[128];
			memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
			sprintf(upgrade_file_path, "%s", writebuf + 1);
			upgrade_file_path[buflen-1] = '\0';
			TPD_DEBUG("%s\n", upgrade_file_path);	
						
			
			
			
			disable_irq(fts_i2c_client->irq);
			ret = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, upgrade_file_path);
			enable_irq(fts_i2c_client->irq);
			if (ret < 0) {
				#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
				dev_err(&fts_i2c_client->dev, "%s:upgrade failed.\n", __func__);
				return ret;
			}
			
			
			
		}
		break;
	
	
	
	case PROC_SET_TEST_FLAG:
		#if FT_ESD_PROTECT
		apk_debug_flag=writebuf[1];
		if(1==apk_debug_flag)
			esd_switch(0);
		else if(0==apk_debug_flag)
			esd_switch(1);
		printk("\n zax flag=%d \n",apk_debug_flag);
		#endif		
		break;
	case PROC_READ_REGISTER:
		writelen = 1;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_WRITE_REGISTER:
		writelen = 2;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_AUTOCLB:
		TPD_DEBUG("%s: autoclb\n", __func__);
		fts_ctpm_auto_clb(fts_i2c_client);
		break;
	case PROC_READ_DATA:
	case PROC_WRITE_DATA:
		printk("fts rawdata PROC_WRITE_DATA");
		writelen = count - 1;
		if(writelen>0)
		{
			ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
			if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
				dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
				return ret;
			}
		}
		break;
	default:
		break;
	}
	
#if FT_ESD_PROTECT
esd_switch(1);apk_debug_flag = 0;
#endif
	return count;
}

static ssize_t fts_debug_read(struct file *filp, char __user *buff, size_t count, loff_t *ppos)
{
	int ret = 0;
	int num_read_chars = 0;
	int readlen = 0;
	u8 regvalue = 0x00, regaddr = 0x00;
	unsigned char buf[READ_BUF_SIZE];
	printk("fts rawdata fts_debug_read\n");
	#if FT_ESD_PROTECT
		esd_switch(0);apk_debug_flag = 1;
	#endif
	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		
		regaddr = 0xA6;
		ret = fts_read_reg(fts_i2c_client, regaddr, &regvalue);
		if (ret < 0)
			num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
		else
			num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
		break;
	case PROC_READ_REGISTER:
		readlen = 1;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
                #if FT_ESD_PROTECT
					
                apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		} 
		num_read_chars = 1;
		break;
	case PROC_READ_DATA:
		printk("fts rawdata PROC_READ_DATA\n");
		readlen = count;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
                #if FT_ESD_PROTECT
				
                apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		}
		
		num_read_chars = readlen;
		break;
	case PROC_WRITE_DATA:
		break;
	default:
		break;
	}
	
	if (copy_to_user(buff, buf, num_read_chars)) {
		dev_err(&fts_i2c_client->dev, "%s:copy to user error\n", __func__);
                #if FT_ESD_PROTECT
					
                apk_debug_flag = 0;
				#endif
		return -EFAULT;
	}
	#if FT_ESD_PROTECT
	
   
	apk_debug_flag = 0;
	#endif
        
	return num_read_chars;
}
static const struct file_operations fts_proc_fops = {
		.owner 	= THIS_MODULE,
		.read 	= fts_debug_read,
		.write 	= fts_debug_write,
		
};
#else
static int fts_debug_write(struct file *filp, 
	const char __user *buff, unsigned long len, void *data)
{
	
	unsigned char writebuf[WRITE_BUF_SIZE];
	int buflen = len;
	int writelen = 0;
	int ret = 0;
	printk("fts rawdata1 fts_debug_write\n");
	#if FT_ESD_PROTECT
			
	apk_debug_flag = 1;
			#endif
	if (copy_from_user(&writebuf, buff, buflen)) {
		dev_err(&fts_i2c_client->dev, "%s:copy from user error\n", __func__);
#if FT_ESD_PROTECT
					
                apk_debug_flag = 0;
				#endif
		return -EFAULT;
	}
	proc_operate_mode = writebuf[0];

	switch (proc_operate_mode) {
	
	case PROC_UPGRADE:
		{
			char upgrade_file_path[128];
			memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
			sprintf(upgrade_file_path, "%s", writebuf + 1);
			upgrade_file_path[buflen-1] = '\0';
			TPD_DEBUG("%s\n", upgrade_file_path);
			
			
			
			disable_irq(fts_i2c_client->irq);
			ret = fts_ctpm_fw_upgrade_with_app_file(fts_i2c_client, upgrade_file_path);
			enable_irq(fts_i2c_client->irq);
			if (ret < 0) {
				dev_err(&fts_i2c_client->dev, "%s:upgrade failed.\n", __func__);
				#if FT_ESD_PROTECT
					
				apk_debug_flag = 0;
				#endif
				return ret;
			}
			
			
			
		}
		break;
	
	
	
	case PROC_SET_TEST_FLAG:
		#if FT_ESD_PROTECT
		apk_debug_flag=writebuf[1];
		if(1==apk_debug_flag)
			
		else if(0==apk_debug_flag)
			
		printk("\n zax flag=%d \n",apk_debug_flag);
		#endif
		break;
	case PROC_READ_REGISTER:
		writelen = 1;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_WRITE_REGISTER:
		writelen = 2;
		ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
		if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_AUTOCLB:
		TPD_DEBUG("%s: autoclb\n", __func__);
		fts_ctpm_auto_clb(fts_i2c_client);
		break;
	case PROC_READ_DATA:
	case PROC_WRITE_DATA:
		printk("fts rawdata1 PROC_WRITE_DATA\n");
		writelen = len - 1;
		if(writelen>0)
		{
			ret = fts_i2c_write(fts_i2c_client, writebuf + 1, writelen);
			if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
				dev_err(&fts_i2c_client->dev, "%s:write iic error\n", __func__);
				return ret;
			}
		}
		break;
	default:
		break;
	}
	
	#if FT_ESD_PROTECT
		
esd_switch(1);apk_debug_flag = 0;
			#endif
	return len;
}

static int fts_debug_read( char *page, char **start,
	off_t off, int count, int *eof, void *data )
{
	
	int ret = 0;
	unsigned char buf[READ_BUF_SIZE];
	int num_read_chars = 0;
	int readlen = 0;
	u8 regvalue = 0x00, regaddr = 0x00;
	printk("fts rawdata1 fts_debug_read\n");
	#if FT_ESD_PROTECT
	
esd_switch(0);apk_debug_flag = 1;
	#endif
	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		
		regaddr = 0xA6;
		ret = fts_read_reg(fts_i2c_client, regaddr, &regvalue);
		if (ret < 0)
			num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
		else
			num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
		break;
	case PROC_READ_REGISTER:
		readlen = 1;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		} 
		num_read_chars = 1;
		break;
	case PROC_READ_DATA:
		printk("fts rawdata1 PROC_READ_DATA\n");
		readlen = count;
		ret = fts_i2c_read(fts_i2c_client, NULL, 0, buf, readlen);
		if (ret < 0) {
#if FT_ESD_PROTECT
					esd_switch(1);apk_debug_flag = 0;
				#endif
			dev_err(&fts_i2c_client->dev, "%s:read iic error\n", __func__);
			return ret;
		}
		
		num_read_chars = readlen;
		break;
	case PROC_WRITE_DATA:
		break;
	default:
		break;
	}
	
	memcpy(page, buf, num_read_chars);
	#if FT_ESD_PROTECT
	
esd_switch(1);apk_debug_flag = 0;
	#endif
	return num_read_chars;
}
#endif
int fts_create_apk_debug_channel(struct i2c_client * client)
{
	#if (1)
		fts_proc_entry = proc_create(PROC_NAME, 0777, NULL, &fts_proc_fops);		
	#else
		fts_proc_entry = create_proc_entry(PROC_NAME, 0777, NULL);
	#endif
	if (NULL == fts_proc_entry) 
	{
		dev_err(&client->dev, "Couldn't create proc entry!\n");
		
		return -ENOMEM;
	} 
	else 
	{
		dev_info(&client->dev, "Create proc entry success!\n");
		
		#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
			
			fts_proc_entry->write_proc = fts_debug_write;
			fts_proc_entry->read_proc = fts_debug_read;
		#endif
	}
	return 0;
}
void fts_release_apk_debug_channel(void)
{
	
	if (fts_proc_entry)
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 10, 0))
			proc_remove(fts_proc_entry);
		#else
			remove_proc_entry(PROC_NAME, NULL);
		#endif
}

static ssize_t fts_tpfwver_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	u8 fwver = 0;
	
	mutex_lock(&fts_input_dev->mutex);
	if (fts_read_reg(fts_i2c_client, FTS_REG_FW_VER, &fwver) < 0)
	{
		E("i2c access error\n");
		return -1;
	}
	
	if (fwver == 255 && fwver == 0)
		num_read_chars = snprintf(buf, PAGE_SIZE,"get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%02X\n", fwver);
	
	mutex_unlock(&fts_input_dev->mutex);
	
	return num_read_chars;
}
static ssize_t fts_tpfwver_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	
	return -EPERM;
}

int isNumCh(const char ch){
	int result = 0;
	
	if(ch >= '0' && ch <= '9'){
		result = 1;
	}
	else if(ch >= 'a' && ch <= 'f'){
		result = 1;
	}
	else if(ch >= 'A' && ch <= 'F'){
		result = 1;
	}
	else{
		result = 0;
	}

	return result;
}

int hexCharToValue(const char ch){
	int result = 0;
	
	if(ch >= '0' && ch <= '9'){
		result = (int)(ch - '0');
	}
	else if(ch >= 'a' && ch <= 'f'){
		result = (int)(ch - 'a') + 10;
	}
	else if(ch >= 'A' && ch <= 'F'){
		result = (int)(ch - 'A') + 10;
	}
	else{
		result = -1;
	}

	return result;
}

int hexToStr(char *hex, int iHexLen, char *ch, int *iChLen)
{
	int high=0;
	int low=0;
	int tmp = 0;
	int i = 0; 
	int iCharLen = 0;
	if(hex == NULL || ch == NULL){
		return -1;
	}

	printk("iHexLen: %d in function:%s\n\n", iHexLen, __func__);

	if(iHexLen %2 == 1){		
		return -2;
	}

	for(i=0; i<iHexLen; i+=2)
	{
		high = hexCharToValue(hex[i]);
		if(high < 0){
			ch[iCharLen] = '\0';
			return -3;
		}

		low = hexCharToValue(hex[i+1]);
		if(low < 0){
			ch[iCharLen] = '\0';
			return -3;
		}
		tmp = (high << 4) + low;
		ch[iCharLen++] = (char)tmp;  
	}
	ch[iCharLen] = '\0';
	*iChLen = iCharLen;
	printk("iCharLen: %d, iChLen: %d in function:%s\n\n", iCharLen, *iChLen, __func__);
	return 0;
}

void strToBytes(char * bufStr, int iLen, char* uBytes, int *iBytesLen)
{
	int i=0;
	int iNumChLen=0;

	*iBytesLen=0;

	for(i=0; i<iLen; i++)
	{		
		if(isNumCh(bufStr[i]))
		{
			bufStr[iNumChLen++] = bufStr[i];
		}
	}

	bufStr[iNumChLen] = '\0';
	
	hexToStr(bufStr, iNumChLen, uBytes, iBytesLen);	
}
static ssize_t fts_tprwreg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	return -EPERM;
}
static ssize_t fts_tprwreg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	int retval;
	
	long unsigned int wmreg=0;
	u8 regaddr=0xff,regvalue=0xff;
	u8 valbuf[5]={0};

	memset(valbuf, 0, sizeof(valbuf));
	mutex_lock(&fts_input_dev->mutex);	
	num_read_chars = count - 1;
	if (num_read_chars != 2) 
	{
		if (num_read_chars != 4) 
		{
			dev_err(dev, "please input 2 or 4 character\n");
			goto error_return;
		}
	}
	memcpy(valbuf, buf, num_read_chars);
	
	strToBytes((char*)buf, num_read_chars, valbuf, &retval);

	if(1==retval) regaddr = valbuf[0];
	else if(2==retval)
		{
		regaddr = valbuf[0];
		regvalue = valbuf[1];
		}
	else retval =0;
	if (0 != retval) 
	{
		dev_err(dev, "%s() - ERROR: Could not convert the given input to a number. The given input was: \"%s\"\n", __FUNCTION__, buf);
		goto error_return;
	}
	if (2 == num_read_chars) 
	{
		
		regaddr = wmreg;
		printk("[focal](0x%02x)\n", regaddr);
		if (fts_read_reg(client, regaddr, &regvalue) < 0)
			printk("[Focal] %s : Could not read the register(0x%02x)\n", __func__, regaddr);
		else
			printk("[Focal] %s : the register(0x%02x) is 0x%02x\n", __func__, regaddr, regvalue);
	} 
	else 
	{
		regaddr = wmreg>>8;
		regvalue = wmreg;
		if (fts_write_reg(client, regaddr, regvalue)<0)
			dev_err(dev, "[Focal] %s : Could not write the register(0x%02x)\n", __func__, regaddr);
		else
			dev_dbg(dev, "[Focal] %s : Write 0x%02x into register(0x%02x) successful\n", __func__, regvalue, regaddr);
	}
	error_return:
	mutex_unlock(&fts_input_dev->mutex);
	
	return count;
}
static ssize_t fts_fwupdate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	return -EPERM;
}

static ssize_t fts_fwupdate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	
	u8 uc_host_fm_ver;
	int i_ret;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	
	#if FT_ESD_PROTECT
		esd_switch(0);apk_debug_flag = 1;
	#endif
	mutex_lock(&fts_input_dev->mutex);
	
	disable_irq(client->irq);

	i_ret = fts_ctpm_fw_upgrade_with_i_file(client);
	if (i_ret == 0)
	{
		msleep(300);
		uc_host_fm_ver = fts_ctpm_get_i_file_ver();
		dev_dbg(dev, "%s [FTS] upgrade to new version 0x%x\n", __func__, uc_host_fm_ver);
	}
	else
	{
		dev_err(dev, "%s ERROR:[FTS] upgrade failed ret=%d.\n", __func__, i_ret);
	}

	
	
	enable_irq(client->irq);
	mutex_unlock(&fts_input_dev->mutex);
	#if FT_ESD_PROTECT
		esd_switch(1);apk_debug_flag = 0;
	#endif
	return count;
}
static ssize_t fts_fwupgradeapp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	return -EPERM;
}

static ssize_t fts_fwupgradeapp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char fwname[128];
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';
	#if FT_ESD_PROTECT
		esd_switch(0);apk_debug_flag = 1;
	#endif
	mutex_lock(&fts_input_dev->mutex);
	
	disable_irq(client->irq);
	fts_ctpm_fw_upgrade_with_app_file(client, fwname);
	enable_irq(client->irq);
	
	mutex_unlock(&fts_input_dev->mutex);
	#if FT_ESD_PROTECT
		esd_switch(1);apk_debug_flag = 0;
	#endif
	return count;
}
static ssize_t fts_getprojectcode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	return -EPERM;
}
static ssize_t fts_getprojectcode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	
	return -EPERM;
}

static DEVICE_ATTR(ftstpfwver, S_IRUGO|S_IWUSR, fts_tpfwver_show, fts_tpfwver_store);
static DEVICE_ATTR(ftsfwupdate, S_IRUGO|S_IWUSR, fts_fwupdate_show, fts_fwupdate_store);
static DEVICE_ATTR(ftstprwreg, S_IRUGO|S_IWUSR, fts_tprwreg_show, fts_tprwreg_store);
static DEVICE_ATTR(ftsfwupgradeapp, S_IRUGO|S_IWUSR, fts_fwupgradeapp_show, fts_fwupgradeapp_store);
static DEVICE_ATTR(ftsgetprojectcode, S_IRUGO|S_IWUSR, fts_getprojectcode_show, fts_getprojectcode_store);



static struct attribute *fts_attributes[] = {
	&dev_attr_ftstpfwver.attr,
	&dev_attr_ftsfwupdate.attr,
	&dev_attr_ftstprwreg.attr,
	&dev_attr_ftsfwupgradeapp.attr,
	&dev_attr_ftsgetprojectcode.attr,
	NULL
};

static struct attribute_group fts_attribute_group = {
	.attrs = fts_attributes
};

int fts_create_sysfs(struct i2c_client * client)
{
	int err;
	err = sysfs_create_group(&client->dev.kobj, &fts_attribute_group);
	if (0 != err) 
	{
		dev_err(&client->dev, "%s() - ERROR: sysfs_create_group() failed.\n", __func__);
		sysfs_remove_group(&client->dev.kobj, &fts_attribute_group);
		return -EIO;
	} 
	else 
	{
		pr_info("fts:%s() - sysfs_create_group() succeeded.\n",__func__);
	}
	
	return err;
}
int fts_remove_sysfs(struct i2c_client * client)
{
	sysfs_remove_group(&client->dev.kobj, &fts_attribute_group);
	return 0;
}