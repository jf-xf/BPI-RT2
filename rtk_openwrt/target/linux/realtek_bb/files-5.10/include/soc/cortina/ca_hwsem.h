#ifndef __CA_HWSEN_H__
#define __CA_HWSEM_H__

typedef enum sem_id{
	CA_SEM_IPC_ARM = 0,
	CA_SEM_IPC_PE0 = 1,
	CA_SEM_IPC_PE1 = 2,
#if defined(CONFIG_ARCH_REALTEK_9607F)
	CA_SEM_IPC_PE2 = 3,
#endif
	CA_SEM_FDB,
	CA_SEM_DUMY1,
	CA_SEM_DUMY2,
}ca_sem_id_t;

/* API prototype */

int ca_sem_lock(ca_sem_id_t semid, unsigned long flags);
int ca_sem_unlock(ca_sem_id_t semid, unsigned long flags);

#endif // __CA_HWSEM_H__

