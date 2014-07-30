#include "pgsampler.h"

char* fs_info(void) {
  char* result;
  FILE *fp;
	struct mntent *fs;
	struct statvfs vfs;
	StringInfoData resultbuf;
	char strbuf[30];

  initStringInfo(&resultbuf);
  appendStringInfo(&resultbuf, "FSINFO;");
  
	fp = setmntent("/etc/mtab", "r");	/* read only */
	if (fp == NULL) {
		elog(LOG, "%s: could not open: %s\n", "/etc/mtab", strerror(errno));
		result = palloc(2);
		strcpy(result, "");
		return result;
	}

	while ((fs = getmntent(fp)) != NULL) {
		if (fs->mnt_fsname[0] == '/') {	
	    if (statvfs(fs->mnt_dir, & vfs) != 0) {
		    //elog(LOG, "SKIPPING %s: statvfs failed: %s\n", fs->mnt_dir, strerror(errno));
	    } else {
	      
	      /* Filesystem Info */
	      appendStringInfoString(&resultbuf, fs->mnt_fsname);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      appendStringInfoString(&resultbuf, fs->mnt_dir);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      appendStringInfoString(&resultbuf, fs->mnt_type);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      appendStringInfoString(&resultbuf, fs->mnt_opts);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      /* Filesystem Stats */
	      sprintf(strbuf, "%ld", (unsigned long) vfs.f_bsize);
	      appendStringInfoString(&resultbuf, strbuf);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      sprintf(strbuf, "%ld", (unsigned long) vfs.f_frsize);
	      appendStringInfoString(&resultbuf, strbuf);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      sprintf(strbuf, "%ld", (unsigned long) vfs.f_blocks);
	      appendStringInfoString(&resultbuf, strbuf);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      sprintf(strbuf, "%ld", (unsigned long) vfs.f_bfree);
	      appendStringInfoString(&resultbuf, strbuf);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      sprintf(strbuf, "%ld", (unsigned long) vfs.f_bavail);
	      appendStringInfoString(&resultbuf, strbuf);
	      appendStringInfo(&resultbuf, FIELD_DELIMIT);
	      
	      appendStringInfo(&resultbuf,REC_DELIMIT);
	    }
	  }
  }

	endmntent(fp);
	// elog(LOG, "FSINFO: %s", resultbuf.data); //print the command to send, debugging
	appendStringInfoString(&resultbuf, CDELIMIT);
  result = pstrdup(resultbuf.data);
	return result; 

}
