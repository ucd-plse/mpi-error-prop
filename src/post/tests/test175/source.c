typedef int umode_t;

struct inner_request {
  int outcome;
};

struct request {
  int result;
  int status;
  struct inner_request* req;
};

struct list_head {
  struct list_head *next, *prev;
};

struct dentry {
  unsigned int d_flags;           /* protected by d_lock */
};

struct inode {
  struct list_head        i_dentry;
  umode_t i_mode;
};


static inline void *ERR_PTR(long error) {
  return (void *) error;
}


void prefetch(struct list_head *);
struct dentry *list_entry(struct list_head *);
int S_ISDIR(umode_t);
int d_unhashed(struct dentry *);
void __dget_locked(struct dentry *);


static struct dentry * __d_find_alias(struct inode *inode, int want_discon) {
  struct list_head *head, *next, *tmp;
  struct dentry *alias, *discon_alias=0;
  
  head = &inode->i_dentry; // dereference
  next = inode->i_dentry.next; // no longer reported
  while (next != head) {
    tmp = next;
    next = tmp->next;
    prefetch(next);
    alias = list_entry(tmp);
    if (S_ISDIR(inode->i_mode) || !d_unhashed(alias)) { // no longer reported
	return alias;
    }
  }

  if (discon_alias)
    __dget_locked(discon_alias);
  return discon_alias;
}

 
int main() {

  struct inode *inode = ERR_PTR(-5);
  __d_find_alias(inode, 1);

  return 0;
}
