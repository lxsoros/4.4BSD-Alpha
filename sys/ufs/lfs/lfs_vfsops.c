/*
 * Copyright (c) 1989, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)lfs_vfsops.c	7.82 (Berkeley) 8/1/92
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/disklabel.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/socket.h>

#include <miscfs/specfs/specdev.h>

#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/ufs_extern.h>

#include <ufs/lfs/lfs.h>
#include <ufs/lfs/lfs_extern.h>

int lfs_mountfs __P((struct vnode *, struct mount *, struct proc *));

struct vfsops lfs_vfsops = {
	lfs_mount,
	ufs_start,
	lfs_unmount,
	lfs_root,
	ufs_quotactl,
	lfs_statfs,
	lfs_sync,
	lfs_vget,
	lfs_fhtovp,
	lfs_vptofh,
	lfs_init,
};

int
lfs_mountroot()
{
	panic("lfs_mountroot");		/* XXX -- implement */
}

/*
 * VFS Operations.
 *
 * mount system call
 */
lfs_mount(mp, path, data, ndp, p)
	register struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
	struct proc *p;
{
	struct vnode *devvp;
	struct ufs_args args;
	struct ufsmount *ump;
	register struct lfs *fs;				/* LFS */
	u_int size;
	int error;

	if (error = copyin(data, (caddr_t)&args, sizeof (struct ufs_args)))
		return (error);

	/* Until LFS can do NFS right.		XXX */
	if (args.exflags & MNT_EXPORTED)
		return (EINVAL);
	/*
	 * If updating, check whether changing from read-only to
	 * read/write; if there is no device name, that's all we do.
	 */
	if (mp->mnt_flag & MNT_UPDATE) {
		ump = VFSTOUFS(mp);
#ifdef NOTLFS							/* LFS */
		fs = ump->um_fs;
		if (fs->fs_ronly && (mp->mnt_flag & MNT_RDONLY) == 0)
			fs->fs_ronly = 0;
#else
		fs = ump->um_lfs;
		if (fs->lfs_ronly && (mp->mnt_flag & MNT_RDONLY) == 0)
			fs->lfs_ronly = 0;
#endif
		if (args.fspec == 0) {
			/*
			 * Process export requests.
			 */
			if (args.exflags & MNT_EXPORTED) {
				if (error = ufs_hang_addrlist(mp, &args))
					return (error);
				mp->mnt_flag |= MNT_EXPORTED;
			}
			if (args.exflags & MNT_DELEXPORT) {
				ufs_free_addrlist(ump);
				mp->mnt_flag &=
				    ~(MNT_EXPORTED | MNT_DEFEXPORTED);
			}
			return (0);
		}
	}
	/*
	 * Not an update, or updating the name: look up the name
	 * and verify that it refers to a sensible block device.
	 */
	NDINIT(ndp, LOOKUP, FOLLOW, UIO_USERSPACE, args.fspec, p);
	if (error = namei(ndp))
		return (error);
	devvp = ndp->ni_vp;
	if (devvp->v_type != VBLK) {
		vrele(devvp);
		return (ENOTBLK);
	}
	if (major(devvp->v_rdev) >= nblkdev) {
		vrele(devvp);
		return (ENXIO);
	}
	if ((mp->mnt_flag & MNT_UPDATE) == 0)
		error = lfs_mountfs(devvp, mp, p);		/* LFS */
	else {
		if (devvp != ump->um_devvp)
			error = EINVAL;	/* needs translation */
		else
			vrele(devvp);
	}
	if (error) {
		vrele(devvp);
		return (error);
	}
	ump = VFSTOUFS(mp);
	fs = ump->um_lfs;					/* LFS */
#ifdef NOTLFS							/* LFS */
	(void) copyinstr(path, fs->fs_fsmnt, sizeof(fs->fs_fsmnt) - 1, &size);
	bzero(fs->fs_fsmnt + size, sizeof(fs->fs_fsmnt) - size);
	bcopy((caddr_t)fs->fs_fsmnt, (caddr_t)mp->mnt_stat.f_mntonname,
	    MNAMELEN);
	(void) copyinstr(args.fspec, mp->mnt_stat.f_mntfromname, MNAMELEN - 1,
	    &size);
	bzero(mp->mnt_stat.f_mntfromname + size, MNAMELEN - size);
	(void) ufs_statfs(mp, &mp->mnt_stat, p);
#else
	(void)copyinstr(path, fs->lfs_fsmnt, sizeof(fs->lfs_fsmnt) - 1, &size);
	bzero(fs->lfs_fsmnt + size, sizeof(fs->lfs_fsmnt) - size);
	bcopy((caddr_t)fs->lfs_fsmnt, (caddr_t)mp->mnt_stat.f_mntonname,
	    MNAMELEN);
	(void) copyinstr(args.fspec, mp->mnt_stat.f_mntfromname, MNAMELEN - 1,
	    &size);
	bzero(mp->mnt_stat.f_mntfromname + size, MNAMELEN - size);
	(void) lfs_statfs(mp, &mp->mnt_stat, p);
#endif
	return (0);
}

/*
 * Common code for mount and mountroot
 * LFS specific
 */
int
lfs_mountfs(devvp, mp, p)
	register struct vnode *devvp;
	struct mount *mp;
	struct proc *p;
{
	extern struct vnode *rootvp;
	register struct lfs *fs;
	register struct ufsmount *ump;
	struct vnode *vp;
	struct buf *bp;
	struct partinfo dpart;
	dev_t dev;
	int error, i, ronly, size;

	/*
	 * Disallow multiple mounts of the same device.
	 * Disallow mounting of a device that is currently in use
	 * (except for root, which might share swap device for miniroot).
	 * Flush out any old buffers remaining from a previous use.
	 */
	if (error = ufs_mountedon(devvp))
		return (error);
	if (vcount(devvp) > 1 && devvp != rootvp)
		return (EBUSY);
	if (error = vinvalbuf(devvp, 1, p->p_ucred, p))
		return (error);

	ronly = (mp->mnt_flag & MNT_RDONLY) != 0;
	if (error = VOP_OPEN(devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED, p))
		return (error);

	if (VOP_IOCTL(devvp, DIOCGPART, (caddr_t)&dpart, FREAD, NOCRED, p) != 0)
		size = DEV_BSIZE;
	else {
		size = dpart.disklab->d_secsize;
#ifdef NEVER_USED
		dpart.part->p_fstype = FS_LFS;
		dpart.part->p_fsize = fs->lfs_fsize;	/* frag size */
		dpart.part->p_frag = fs->lfs_frag;	/* frags per block */
		dpart.part->p_cpg = fs->lfs_segshift;	/* segment shift */
#endif
	}

	/* Don't free random space on error. */
	bp = NULL;
	ump = NULL;

	/* Read in the superblock. */
	if (error = bread(devvp, LFS_LABELPAD / size, LFS_SBPAD, NOCRED, &bp))
		goto out;
	fs = bp->b_un.b_lfs;

	/* Check the basics. */
	if (fs->lfs_magic != LFS_MAGIC || fs->lfs_bsize > MAXBSIZE ||
	    fs->lfs_bsize < sizeof(struct lfs)) {
		error = EINVAL;		/* XXX needs translation */
		goto out;
	}
#ifdef DEBUG
	lfs_dump_super(fs);
#endif

	/* Allocate the mount structure, copy the superblock into it. */
	ump = (struct ufsmount *)malloc(sizeof *ump, M_UFSMNT, M_WAITOK);
	fs = ump->um_lfs = malloc(sizeof(struct lfs), M_UFSMNT, M_WAITOK);
	bcopy(bp->b_un.b_addr, fs, sizeof(struct lfs));
	if (sizeof(struct lfs) < LFS_SBPAD)			/* XXX why? */
		bp->b_flags |= B_INVAL;
	brelse(bp);
	bp = NULL;

	/* Set up the I/O information */
	fs->lfs_iocount = 0;

	/* Set up the ifile and lock aflags */
	fs->lfs_doifile = 0;
	fs->lfs_writer = 0;
	fs->lfs_dirops = 0;
	fs->lfs_seglock = 0;

	/* Set the file system readonly/modify bits. */
	fs->lfs_ronly = ronly;
	if (ronly == 0)
		fs->lfs_fmod = 1;

	/* Initialize the mount structure. */
	dev = devvp->v_rdev;
	mp->mnt_data = (qaddr_t)ump;
	mp->mnt_stat.f_fsid.val[0] = (long)dev;
	mp->mnt_stat.f_fsid.val[1] = MOUNT_LFS;
	mp->mnt_flag |= MNT_LOCAL;
	ump->um_mountp = mp;
	ump->um_dev = dev;
	ump->um_devvp = devvp;
	for (i = 0; i < MAXQUOTAS; i++)
		ump->um_quotas[i] = NULLVP;
	devvp->v_specflags |= SI_MOUNTEDON;

	/*
	 * We use the ifile vnode for almost every operation.  Instead of
	 * retrieving it from the hash table each time we retrieve it here,
	 * artificially increment the reference count and keep a pointer
	 * to it in the incore copy of the superblock.
	 */
	if (error = VFS_VGET(mp, LFS_IFILE_INUM, &vp))
		goto out;
	fs->lfs_ivnode = vp;
	VREF(vp);
	vput(vp);

	return (0);
out:
	if (bp)
		brelse(bp);
	(void)VOP_CLOSE(devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED, p);
	if (ump) {
		free(ump->um_lfs, M_UFSMNT);
		free(ump, M_UFSMNT);
		mp->mnt_data = (qaddr_t)0;
	}
	return (error);
}

/*
 * unmount system call
 */
lfs_unmount(mp, mntflags, p)
	struct mount *mp;
	int mntflags;
	struct proc *p;
{
	extern int doforce;
	register struct ufsmount *ump;
	register struct lfs *fs;
	int i, error, flags, ronly;

	flags = 0;
	if (mntflags & MNT_FORCE) {
		if (!doforce || mp == rootfs)
			return (EINVAL);
		flags |= FORCECLOSE;
	}

	ump = VFSTOUFS(mp);
	fs = ump->um_lfs;
#ifdef QUOTA
	if (mp->mnt_flag & MNT_QUOTA) {
		if (error = vflush(mp, fs->lfs_ivnode, SKIPSYSTEM|flags))
			return (error);
		for (i = 0; i < MAXQUOTAS; i++) {
			if (ump->um_quotas[i] == NULLVP)
				continue;
			quotaoff(p, mp, i);
		}
		/*
		 * Here we fall through to vflush again to ensure
		 * that we have gotten rid of all the system vnodes.
		 */
	}
#endif
	if (error = vflush(mp, fs->lfs_ivnode, flags))
		return (error);
	fs->lfs_clean = 1;
	if (error = VFS_SYNC(mp, 1, p->p_ucred, p))
		return (error);
	if (fs->lfs_ivnode->v_dirtyblkhd)
		panic("lfs_unmount: still dirty blocks on ifile vnode\n");
	vrele(fs->lfs_ivnode);
	vgone(fs->lfs_ivnode);

	ronly = !fs->lfs_ronly;
	ump->um_devvp->v_specflags &= ~SI_MOUNTEDON;
	error = VOP_CLOSE(ump->um_devvp,
	    ronly ? FREAD : FREAD|FWRITE, NOCRED, p);
	vrele(ump->um_devvp);
	free(fs, M_UFSMNT);
	free(ump, M_UFSMNT);
	mp->mnt_data = (qaddr_t)0;
	mp->mnt_flag &= ~MNT_LOCAL;
	return (error);
}

/*
 * Return root of a filesystem
 */
int
lfs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	struct vnode *nvp;
	int error;

	if (error = VFS_VGET(mp, (ino_t)ROOTINO, &nvp))
		return (error);
	*vpp = nvp;
	return (0);
}

/*
 * Get file system statistics.
 */
lfs_statfs(mp, sbp, p)
	struct mount *mp;
	register struct statfs *sbp;
	struct proc *p;
{
	register struct lfs *fs;
	register struct ufsmount *ump;

	ump = VFSTOUFS(mp);
	fs = ump->um_lfs;
	if (fs->lfs_magic != LFS_MAGIC)
		panic("lfs_statfs: magic");
	sbp->f_type = MOUNT_LFS;
	sbp->f_bsize = fs->lfs_bsize;
	sbp->f_iosize = fs->lfs_bsize;
	sbp->f_blocks = fs->lfs_dsize;
	sbp->f_bfree = dbtofsb(fs, fs->lfs_bfree);
	sbp->f_bavail = (fs->lfs_dsize * (100 - fs->lfs_minfree) / 100) -
		(fs->lfs_dsize - sbp->f_bfree);
	sbp->f_files = fs->lfs_nfiles;
	sbp->f_ffree = sbp->f_bfree * INOPB(fs);
	if (sbp != &mp->mnt_stat) {
		bcopy((caddr_t)mp->mnt_stat.f_mntonname,
			(caddr_t)&sbp->f_mntonname[0], MNAMELEN);
		bcopy((caddr_t)mp->mnt_stat.f_mntfromname,
			(caddr_t)&sbp->f_mntfromname[0], MNAMELEN);
	}
	return (0);
}

/*
 * Go through the disk queues to initiate sandbagged IO;
 * go through the inodes to write those that have been modified;
 * initiate the writing of the super block if it has been modified.
 *
 * Note: we are always called with the filesystem marked `MPBUSY'.
 */
lfs_sync(mp, waitfor, cred, p)
	struct mount *mp;
	int waitfor;
	struct ucred *cred;
	struct proc *p;
{
	extern int syncprt;
	int error;

	/* All syncs must be checkpoints until roll-forward is implemented. */
	error = lfs_segwrite(mp, 1);
#ifdef QUOTA
	qsync(mp);
#endif
	return (error);
}

/*
 * Look up an LFS dinode number to find its incore vnode.  If not already
 * in core, read it in from the specified device.  Return the inode locked.
 * Detection and handling of mount points must be done by the calling routine.
 */
int
lfs_vget(mp, ino, vpp)
	struct mount *mp;
	ino_t ino;
	struct vnode **vpp;
{
	register struct lfs *fs;
	register struct inode *ip;
	struct buf *bp;
	struct ifile *ifp;
	struct vnode *vp;
	struct ufsmount *ump;
	daddr_t daddr;
	dev_t dev;
	int error;

	ump = VFSTOUFS(mp);
	dev = ump->um_dev;
	if ((*vpp = ufs_ihashget(dev, ino)) != NULL)
		return (0);

	/* Translate the inode number to a disk address. */
	fs = ump->um_lfs;
	if (ino == LFS_IFILE_INUM)
		daddr = fs->lfs_idaddr;
	else {
		LFS_IENTRY(ifp, fs, ino, bp);
		daddr = ifp->if_daddr;
		brelse(bp);
		if (daddr == LFS_UNUSED_DADDR)
			return (ENOENT);
	}

	/* Allocate new vnode/inode. */
	if (error = lfs_vcreate(mp, ino, &vp)) {
		*vpp = NULL;
		return (error);
	}

	/*
	 * Put it onto its hash chain and lock it so that other requests for
	 * this inode will block if they arrive while we are sleeping waiting
	 * for old data structures to be purged or for the contents of the
	 * disk portion of this inode to be read.
	 */
	ip = VTOI(vp);
	ufs_ihashins(ip);

	/*
	 * XXX
	 * This may not need to be here, logically it should go down with
	 * the i_devvp initialization.
	 * Ask Kirk.
	 */
	ip->i_lfs = ump->um_lfs;

	/* Read in the disk contents for the inode, copy into the inode. */
	if (error =
	    bread(ump->um_devvp, daddr, (int)fs->lfs_bsize, NOCRED, &bp)) {
		/*
		 * The inode does not contain anything useful, so it
		 * would be misleading to leave it on its hash chain.
		 * Iput() will return it to the free list.
		 */
		ufs_ihashrem(ip);

		/* Unlock and discard unneeded inode. */
		ufs_iput(ip);
		brelse(bp);
		*vpp = NULL;
		return (error);
	}
	ip->i_din = *lfs_ifind(fs, ino, bp->b_un.b_dino);
	brelse(bp);

	/*
	 * Initialize the vnode from the inode, check for aliases.  In all
	 * cases re-init ip, the underlying vnode/inode may have changed.
	 */
	if (error = ufs_vinit(mp, lfs_specop_p, LFS_FIFOOPS, &vp)) {
		ufs_iput(ip);
		*vpp = NULL;
		return (error);
	}
	/*
	 * Finish inode initialization now that aliasing has been resolved.
	 */
	ip->i_devvp = ump->um_devvp;
	VREF(ip->i_devvp);
	*vpp = vp;
	return (0);
}

/*
 * File handle to vnode
 *
 * Have to be really careful about stale file handles:
 * - check that the inode number is valid
 * - call lfs_vget() to get the locked inode
 * - check for an unallocated inode (i_mode == 0)
 * - check that the generation number matches
 *
 * XXX
 * use ifile to see if inode is allocated instead of reading off disk
 * what is the relationship between my generational number and the NFS
 * generational number.
 */
int
lfs_fhtovp(mp, fhp, nam, vpp, exflagsp, credanonp)
	register struct mount *mp;
	struct fid *fhp;
	struct mbuf *nam;
	struct vnode **vpp;
	int *exflagsp;
	struct ucred **credanonp;
{
	register struct inode *ip;
	register struct ufid *ufhp;
	register struct netaddrhash *np;
	register struct ufsmount *ump = VFSTOUFS(mp);
	struct vnode *nvp;
	struct sockaddr *saddr;
	int error;

	ufhp = (struct ufid *)fhp;
	if (ufhp->ufid_ino < ROOTINO)
		return (ESTALE);
	/*
	 * Get the export permission structure for this <mp, client> tuple.
	 */
	if ((mp->mnt_flag & MNT_EXPORTED) == 0)
		return (EACCES);
	if (nam == NULL) {
		np = (struct netaddrhash *)0;
	} else {
		/*
		 * First search for a network match.
		 */
		np = ump->um_netaddr[NETMASK_HASH];
		while (np) {
			if (netaddr_match(np->neth_family, &np->neth_haddr,
			    &np->neth_hmask, nam))
				break;
			np = np->neth_next;
		}

		/*
		 * If not found, try for an address match.
		 */
		if (np == (struct netaddrhash *)0) {
			saddr = mtod(nam, struct sockaddr *);
			np = ump->um_netaddr[NETADDRHASH(saddr)];
			while (np) {
				if (netaddr_match(np->neth_family,
				    &np->neth_haddr, (struct netaddrhash *)0,
				    nam))
					break;
				np = np->neth_next;
			}
		}
	}
	if (np == (struct netaddrhash *)0) {
		/*
		 * If no address match, use the default if it exists.
		 */
		if ((mp->mnt_flag & MNT_DEFEXPORTED) == 0)
			return (EACCES);
		np = &ump->um_defexported;
	}
	if (error = VFS_VGET(mp, ufhp->ufid_ino, &nvp)) {
		*vpp = NULLVP;
		return (error);
	}
	ip = VTOI(nvp);
	if (ip->i_mode == 0 || ip->i_gen != ufhp->ufid_gen) {
		ufs_iput(ip);
		*vpp = NULLVP;
		return (ESTALE);
	}
	*vpp = nvp;
	*exflagsp = np->neth_exflags;
	*credanonp = &np->neth_anon;
	return (0);
}

/*
 * Vnode pointer to File handle
 */
/* ARGSUSED */
lfs_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{
	register struct inode *ip;
	register struct ufid *ufhp;

	ip = VTOI(vp);
	ufhp = (struct ufid *)fhp;
	ufhp->ufid_len = sizeof(struct ufid);
	ufhp->ufid_ino = ip->i_number;
	ufhp->ufid_gen = ip->i_gen;
	return (0);
}
