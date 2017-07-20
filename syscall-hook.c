#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/utsname.h>

MODULE_LICENSE("GPL");

static void **syscall_table = (void *)__SYSCALL_TABLE_ADDRESS__;

asmlinkage int (*org_open)(const char *pathname, int flags);
asmlinkage static int hook_open(const char *pathname, int flags) {
	printk(KERN_INFO "[*] hook_open(\"%s\" %d)\n", pathname, flags);
	return org_open(pathname, flags);
}

static void save_original_syscall_address(void)
{
    org_open = syscall_table[__NR_open];
}

static void change_page_attr_to_rw(pte_t *pte)
{
    set_pte_atomic(pte, pte_mkwrite(*pte));
}

static void change_page_attr_to_ro(pte_t *pte)
{
    set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
}

static void replace_system_call(void *new)
{
    unsigned int level = 0;
    pte_t *pte;

    pte = lookup_address((unsigned long) syscall_table, &level);
    /* Need to set r/w to a page which syscall_table is in. */
    change_page_attr_to_rw(pte);

    syscall_table[__NR_open] = new;
    /* set back to read only */
    change_page_attr_to_ro(pte);
}

static int hook_init(void) {
	printk(KERN_INFO "[*] hook_init\n");
	printk(KERN_INFO "[*] syscall_table_address is 0x%p\n", syscall_table);

	save_original_syscall_address();
	printk(KERN_INFO "[*] orogial sys_oepn address is 0x%p\n", org_open);

	replace_system_call(hook_open);

	printk(KERN_INFO "[*] system call replaced\n");

	return 0;

}

static void hook_exit(void) {
	printk(KERN_INFO "[*] hook_exit\n");
	syscall_table[__NR_open] = org_open;
}

module_init(hook_init);
module_exit(hook_exit);
