/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "mount.h"

bool_t
xdr_fhandle(xdrs, objp)
	register XDR *xdrs;
	fhandle objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_opaque(xdrs, objp, FHSIZE))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_fhstatus(xdrs, objp)
	register XDR *xdrs;
	fhstatus *objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_u_int(xdrs, &objp->fhs_status))
		return (FALSE);
	switch (objp->fhs_status) {
	case 0:
		if (!xdr_fhandle(xdrs, objp->fhstatus_u.fhs_fhandle))
			return (FALSE);
		break;
	}
	return (TRUE);
}

bool_t
xdr_dirpath(xdrs, objp)
	register XDR *xdrs;
	dirpath *objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_string(xdrs, objp, MNTPATHLEN))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_name(xdrs, objp)
	register XDR *xdrs;
	name *objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_string(xdrs, objp, MNTNAMLEN))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_mountlist(xdrs, objp)
	register XDR *xdrs;
	mountlist *objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_name(xdrs, &objp->ml_hostname))
		return (FALSE);
	if (!xdr_dirpath(xdrs, &objp->ml_directory))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->ml_next, sizeof (mountlist), (xdrproc_t) xdr_mountlist))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_groups(xdrs, objp)
	register XDR *xdrs;
	groups *objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_pointer(xdrs, (char **)objp, sizeof (struct groupnode), (xdrproc_t) xdr_groupnode))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_groupnode(xdrs, objp)
	register XDR *xdrs;
	groupnode *objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_name(xdrs, &objp->gr_name))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->gr_next, sizeof (groups), (xdrproc_t) xdr_groups))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_exports(xdrs, objp)
	register XDR *xdrs;
	exports *objp;
{

#if defined(_LP64) || defined(_KERNEL)
	register int *buf;
#else
	register long *buf;
#endif

	if (!xdr_dirpath(xdrs, &objp->ex_dir))
		return (FALSE);
	if (!xdr_groups(xdrs, &objp->ex_groups))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->ex_next, sizeof (exports), (xdrproc_t) xdr_exports))
		return (FALSE);
	return (TRUE);
}
