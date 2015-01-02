#ifndef MF_MAILDIR_H
#define MF_MAILDIR_H

/**
 * Set up the Maildir delivery system.
 */
int maildir_init(void);


/**
 * Deliver a message to a maildir as a hard link.
 */
int maildir_deliver_link(const char *src, const char *mdir, char **out_fn);

#endif
