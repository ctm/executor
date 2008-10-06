enum { DRVR_MINUMUM = 12, DRVR_MAXIMUM = 31 };

/*
 * setup_map makes sure all resources that need to be copied can have a
 * proper mapping.  If it succeeds, enough information is stored in *mappingp
 * so that the mapping doesn't need to be done again.
 */

OSErr setup_map (INTEGER from_rn, INTEGER master_rn, mapping_t *mappingp)
{
  return noErr; /* TODO */
}

BOOLEAN map_id (INTEGER *idp, INTEGER from_rn, INTEGER master_rn,
		const mapping_t *mappingp)
{
  return true; /* TODO */
}

void map_data (Handle h, const mapping_t *mappingp)
{
  /* TODO */
}

INTEGER CountTypesRN (INTEGER rn)
{
  INTEGER savern, retval;

  savern = CurResFile();
  UseResFile(rn);
  retval = Count1Types();
  UseResFile(savern);
  return retval;
}

void GetIndTypeRN ( INTEGER rn, ResType *typep, INTEGER type_num)
{
  INTEGER savern;

  savern = CurResFile();
  UseResFile(rn);
  Get1IndType (typep, type_num);
  UseResFile(savern);
}

INTEGER CountResourcesRN (INTEGER rn, ResType type)
{
  INTEGER savern, retval;

  savern = CurResFile();
  UseResFile(rn);
  retval = Count1Resources(type);
  UseResFile(savern);
  return retval;
}

void AddResourceRN ( INTEGER rn, Handle h, ResType type, INTEGER id,
		    Str255 name)
{
  INTEGER savern;

  savern = CurResFile();
  UseResFile(rn);
  AddResource (h, type, id, name);
  UseResFile(savern);
}

OSErr AddToMasterFile(INTEGER from_file_rn, INTEGER master_file_rn)
{
  OSErr err;
  mapping_t mapping;
  INTEGER type_num, type_num_max;
  ResType type;
  Handle h;
  Str255 name;
  BOOLEAN save_resload;

  err = setup_map (from_file_rn, master_file_rn, &mapping);
  if (err == noEErr)
    {
      type_num_max = CountTypesRN (from_file_rn);
      save_resload = ResLoad;
      SetResLoad (false);
      for (type_num = 1; type_num <= type_num_max; ++type_num)
	{
	  GetIndTypeRN (from_file_rn, &type, type_num);
	  res_num_max = CountResourcesRN (from_file_rn, type);
	  for (res_num = 1; res_num <= res_num_max; ++res_num)
	    {
	      h = GetIndResourceRN (from_file_rn, type, res_num);
	      GetResInfo (h, &id, &t, name);
	      if (map_id (&id, from_file_rn, master_file_rn, &mapping))
		{
		  LoadResource(h);
		  DetachResource(h);
		  map_data (h, &mapping);
		  AddResourceRN (master_file_rn, h, type, id, name);
		}
	    }
	}
      SetResLoad (save_resload);
    }
}

typedef struct link_str
{
  link_str *next;
  Handle resource;
} link_t;

OSErr RemoveFromMasterFile(INTEGER from_file_rn, INTEGER master_file_rn)
{
  link_t *headp;
  OSErr err;

  err = setup_map (from_file_rn, master_file_rn, &mapping);
  if (err == noErr)
    {
      type_num_max = CountTypesRN (master_file_rn);
      save_resload = ResLoad;
      SetResLoad (false);
      headp = 0;
      for (type_num = 1; type_num <= type_num_max; ++type_num)
	{
	  GetIndTypeRN (master_file_rn, &type, type_num);
	  res_num_max = CountResourcesRN (master_file_rn, type);
	  for (res_num = 1; res_num <= res_num_max; ++res_num)
	    {
	      h = GetIndResourceRN (master_file_rn, type, res_num);
	      GetResInfo (h, &id, &t, name);
	      if (??? (id, &mapping))
		{
		  link_t *newlinkp;
		  newlinkp = (link_t *) NewPtr(sizeof(link_t));
		  newlinkp->next = headp;
		  newlinkp->resource = h;
		  headp = newlinkp;
		}
	    }
	}
      while (headp)
	{
	  oldp = headp;
	  headp = headp->next;
	  RmveResource(oldp->resource);
	  DisposPtr(oldp);
	}
    }
  return err;
}

