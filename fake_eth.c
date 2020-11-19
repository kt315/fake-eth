/**
 * @file fake_eth.c
 * @author Sergey Kraev [kt315] (kt315@tagan.ru)
 * @brief 
 * @version 0.1
 * @date 18/11/2020
 * 
 * @copyright PG19 (c) 2020
 * 
 */
#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "fake_eth"
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>

#include <linux/printk.h>

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>

#include <linux/proc_fs.h>
#include <linux/fs.h>

#define MAX_BUFF 100
#define MAX_FAKE_DEV 10

MODULE_AUTHOR("kt315@tagan.ru");
MODULE_LICENSE("GPL");

typedef enum isActive_t
{
    FALSE,
    TRUE
} isActive_t;

struct fake_eth_t
{
    // u8 number;
    struct net_device *dev;
    isActive_t isActive;
    char name_fake_eth[16];
    u8 mac_addr[ETH_ALEN];
};

// declare some func
int my_register_netdev(struct fake_eth_t *);
void my_deregister_netdev(struct fake_eth_t *);
void my_dereg_all_netdev(void);

static struct fake_eth_t devs[MAX_FAKE_DEV];

void clear_fake_eth_struct(struct fake_eth_t *fake_eth)
{
    fake_eth->dev = NULL;
    fake_eth->isActive = FALSE;
    memset(fake_eth->mac_addr, 0x0, ETH_ALEN);
    memset(fake_eth->name_fake_eth, 0x0, 16);
}

void devs_init(void)
{
    u8 i;
    for (i = 0; i < MAX_FAKE_DEV; i++)
    {
        clear_fake_eth_struct(&devs[i]);
    }
}

ssize_t read_pfs(struct file *filp, char __user *user_data, size_t sz, loff_t *off)
{
    int len = 0;
    char tmp[1000] = "\0";
    len += sprintf(tmp + len, "#for add eth write:\n");
    len += sprintf(tmp + len, "#  +fake%%d;00:12:13:14:15:16\n");
    len += sprintf(tmp + len, "#for remove eth write: \n");
    len += sprintf(tmp + len, "#  -fake0\n\n");
    len += sprintf(tmp + len, "#current state:\n");
    int j;
    for (j = 0; j < MAX_FAKE_DEV; j++)
    {
        if (devs[j].isActive == TRUE &&
            devs[j].dev != NULL)
        {
            len += sprintf(tmp + len,
                           "%s;%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                           devs[j].dev->name,
                           devs[j].dev->dev_addr[0], devs[j].dev->dev_addr[1], devs[j].dev->dev_addr[2],
                           devs[j].dev->dev_addr[3], devs[j].dev->dev_addr[4], devs[j].dev->dev_addr[5]);
        }
    }

    return simple_read_from_buffer(user_data, sz, off, tmp, len);
}

ssize_t write_pfs(struct file *filp, const char __user *user_data, size_t sz, loff_t *off)
{
    // resive text from userspace
    char tmp[MAX_BUFF] = "\0";
    ssize_t res = simple_write_to_buffer(tmp, MAX_BUFF - 1, off, user_data, sz);
    // pr_info("resived: %d\n", res);
    // pr_info("%.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n",
    //         tmp[0], tmp[1], tmp[2], tmp[3], tmp[4],
    //         tmp[5], tmp[6], tmp[7], tmp[8], tmp[9]);
    if (tmp[res - 1] != ';')
    {
        tmp[res - 1] = ';';
        tmp[res] = '\n';
    }
    // select action: add device
    if (tmp[0] == '+')
    {
        char *resived_string_ptr = NULL;
        char *current_str_ptr = NULL;
        int position_in_resieved_string = 0;
        struct fake_eth_t *select_fake_eth = NULL;

        // select free slot
        int i;
        for (i = 0; i < MAX_FAKE_DEV; i++)
        {
            if (devs[i].isActive == FALSE)
            {
                select_fake_eth = &devs[i];
                break;
            }
        }
        // if not free slots
        if (select_fake_eth == NULL)
        {
            pr_info("not addided. maybe there are no free slots.\n");
            goto finish;
        }

        // parse string
        resived_string_ptr = ((char *)(&tmp)) + 1;
        while ((current_str_ptr = strsep(&resived_string_ptr, ";")) != NULL)
        {
            switch (position_in_resieved_string)
            {
            case 0:
            {
                strcpy(select_fake_eth->name_fake_eth, current_str_ptr);
                pr_info("select_fake_eth->name_fake_eth: %s\n", select_fake_eth->name_fake_eth);
            }
            break;
            case 1:
            {
                if (!mac_pton(current_str_ptr, select_fake_eth->mac_addr))
                {
                    pr_info("err mac parse\n");
                    goto finish;
                }
                pr_info("select_fake_eth->mac_addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                        select_fake_eth->mac_addr[0],
                        select_fake_eth->mac_addr[1],
                        select_fake_eth->mac_addr[2],
                        select_fake_eth->mac_addr[3],
                        select_fake_eth->mac_addr[4],
                        select_fake_eth->mac_addr[5]);
            }
            break;
            default:
                break;
            }
            position_in_resieved_string++;
        }
        my_register_netdev(select_fake_eth);
    }
    // select action: remove device
    else if (tmp[0] == '-')
    {
        char *resived_string_ptr = NULL;
        char *current_str_ptr = NULL;
        int position_in_resieved_string = 0;
        struct fake_eth_t *select_fake_eth = NULL;

        resived_string_ptr = ((char *)(&tmp)) + 1;
        while ((current_str_ptr = strsep(&resived_string_ptr, ";")) != NULL)
        {
            switch (position_in_resieved_string)
            {
            case 0:
            {
                u8 i;
                for (i = 0; i < MAX_FAKE_DEV; i++)
                {
                    if (devs[i].isActive == TRUE &&
                        devs[i].dev != NULL &&
                        strcmp(devs[i].dev->name, current_str_ptr) == 0)
                    {
                        select_fake_eth = &devs[i];
                        break;
                    }
                }
            }
            break;
            default:
                break;
            }
            position_in_resieved_string++;
        }
        if (select_fake_eth != NULL)
        {
            // pr_info("found: %s\n", select_fake_eth->dev->name);
            my_deregister_netdev(select_fake_eth);
        }
        else
        {
            pr_info("not found: %s\n", tmp);
        }
        goto finish;
    }
    // some strange
    else
    {
        pr_info("Garbage in srting: %s", tmp);
        goto finish;
    }
finish:
    return res;
}

struct file_operations proc_read = {.owner = THIS_MODULE, .read = read_pfs, .write = write_pfs};

/**
 * for netdevice 
 */
static int my_open(struct net_device *dev)
{
    netif_start_queue(dev);
    return 0;
}

static int my_close(struct net_device *dev)
{
    netif_stop_queue(dev);
    return 0;
}

static int stub_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    dev_kfree_skb(skb);
    return 0;
}

static struct net_device_ops ndo = {
    .ndo_open = my_open,
    .ndo_stop = my_close,
    .ndo_start_xmit = stub_start_xmit,
};

int my_register_netdev(struct fake_eth_t *fake_eth)
{
    // create dev in network system
    fake_eth->dev = alloc_netdev(0, fake_eth->name_fake_eth, NET_NAME_ENUM, ether_setup);

    memcpy(fake_eth->dev->dev_addr, fake_eth->mac_addr, ETH_ALEN);
    fake_eth->dev->netdev_ops = &ndo;
    netif_carrier_off(fake_eth->dev);
    if (register_netdev(fake_eth->dev))
    {
        pr_info("Failed to register\n");
        free_netdev(fake_eth->dev);
        clear_fake_eth_struct(fake_eth);
        return -1;
    }
    fake_eth->isActive = TRUE;
    pr_info("Registered %s\n", fake_eth->dev->name);
    return 0;
}

void my_deregister_netdev(struct fake_eth_t *fake_eth)
{
    pr_info("dereg: %s\n", fake_eth->dev->name);
    unregister_netdev(fake_eth->dev);
    free_netdev(fake_eth->dev);
    clear_fake_eth_struct(fake_eth);
}

void my_dereg_all_netdev(void)
{
    int i;
    for (i = 0; i < MAX_FAKE_DEV; i++)
    {
        if (devs[i].isActive == TRUE && devs[i].dev != NULL)
        {
            my_deregister_netdev(&devs[i]);
        }
    }
}

static int __init my_init(void)
{
    pr_info("Loading fake_eth!\n");
    devs_init();
    proc_create(KBUILD_MODNAME, 0, NULL, &proc_read);
    return 0;
}

static void __exit my_exit(void)
{
    my_dereg_all_netdev();
    remove_proc_entry(KBUILD_MODNAME, NULL);
    pr_info("Unloading fake_eth!\n");
}

module_init(my_init);
module_exit(my_exit);