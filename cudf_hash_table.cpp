
/*******************************************************/
/* CUDF solver: cudf_hash_table.c                      */
/* Handling hash tables to get a virtual package from  */
/* it name                                             */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// CUDF hash table capabilities
// used to get a virtual package from its name

#include <cudf_hash_table.h>
#ifdef HASH_STRING
#include <string>
#endif

an_hash_table cudf_packages(5);
int virtual_package_rank = 0;



// return a virtual package knowing its name
#ifdef HASH_STRING

CUDFVirtualPackage *get_virtual_package(const char *pkgname) {
  CUDFVirtualPackage *pkg;
  an_hash_table_iterator pkg_item;
  string hash_name = string(pkgname);

  pkg_item = cudf_packages.find(hash_name);

  if (pkg_item == cudf_packages.end()) {
    // printf("get_virtual_package create \"%s\"\n", hash_name);
    pkg = new CUDFVirtualPackage(pkgname, virtual_package_rank++);
    all_virtual_packages.push_back(pkg);
    cudf_packages[hash_name] = pkg;
  } else {
    // printf("get_virtual_package reuse  \"%s\"\n", hash_name);
    pkg = pkg_item->second;
  }
  
  if (strcmp(pkgname, pkg->name) != 0) {
    int lgth;
    printf("NAME CLASH FOR \"%s\" : got \"%s\"\n", pkgname, pkg->name);
    lgth = strlen(pkgname);
    for (int i = 0; i < lgth; i++) 
      if (pkgname[i] < 32)
	printf("\\%2d", pkgname[i]);
      else
	printf("%c", pkgname[i]);
    printf("\n");
    lgth = strlen(pkg->name);
    for (int i = 0; i < lgth; i++) 
      if (pkgname[i] < 32)
	printf("\\%2d", pkg->name[i]);
      else
	printf("%c", pkg->name[i]);
    printf("\n");
    exit(1);
  }

  return pkg;
}

#else

char hash_name[1024]; /* Always use the same buffer for hash table ... otherwise it stucks ... */

CUDFVirtualPackage *get_virtual_package(const char *pkgname) {
  CUDFVirtualPackage *pkg;
  an_hash_table_iterator pkg_item;

  strcpy(hash_name, pkgname);
  //  printf("Handling 2 %s.\n", pkgname);
  pkg_item = cudf_packages.find(hash_name);

  if (pkg_item == cudf_packages.end()) {
    printf("get_virtual_package create \"%s\"\n", hash_name);
    pkg = new CUDFVirtualPackage(hash_name, virtual_package_rank++);
    all_virtual_packages.push_back(pkg);
    cudf_packages[hash_name] = pkg;
  } else {
    printf("get_virtual_package reuse  \"%s\"\n", hash_name);
    pkg = pkg_item->second;
  }

  if (strcmp(pkgname, pkg->name) != 0) {
    int lgth;
    printf("NAME CLASH FOR \"%s\" : got \"%s\"\n", pkgname, pkg->name);
    lgth = strlen(pkgname);
    for (int i = 0; i < lgth; i++) 
      if (pkgname[i] < 32)
	printf("\\%2d", pkgname[i]);
      else
	printf("%c", pkgname[i]);
    printf("\n");
    lgth = strlen(pkg->name);
    for (int i = 0; i < lgth; i++) 
      if (pkgname[i] < 32)
	printf("\\%2d", pkg->name[i]);
      else
	printf("%c", pkg->name[i]);
    printf("\n");
    exit(1);
  }

  return pkg;
}

CUDFVirtualPackage *get_virtual_package_old(const char *pkgname) {
  CUDFVirtualPackage *pkg;
  an_hash_table_iterator pkg_item;

  pkg_item = cudf_packages.find(pkgname);

  if (pkg_item == cudf_packages.end()) {
    printf("get_virtual_package create \"%s\"\n", pkgname);
    pkg = new CUDFVirtualPackage(pkgname, virtual_package_rank++);
    all_virtual_packages.push_back(pkg);
    cudf_packages[pkgname] = pkg;
  } else {
    printf("get_virtual_package reuse  \"%s\"\n", pkgname);
    pkg = pkg_item->second;
  }

  if (strcmp(pkgname, pkg->name) != 0) {
    int lgth;
    printf("NAME CLASH FOR \"%s\" : got \"%s\"\n", pkgname, pkg->name);
    lgth = strlen(pkgname);
    for (int i = 0; i < lgth; i++) 
      if (pkgname[i] < 32)
	printf("\\%2d", pkgname[i]);
      else
	printf("%c", pkgname[i]);
    printf("\n");
    lgth = strlen(pkg->name);
    for (int i = 0; i < lgth; i++) 
      if (pkgname[i] < 32)
	printf("\\%2d", pkg->name[i]);
      else
	printf("%c", pkg->name[i]);
    printf("\n");
    exit(1);
  }

  return pkg;
}

#endif
