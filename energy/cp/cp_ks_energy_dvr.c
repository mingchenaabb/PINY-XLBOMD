/*==========================================================================*/
/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/
/*==========================================================================*/
/*                                                                          */
/*                         PI_MD:                                           */
/*             The future of simulation technology                          */
/*             ------------------------------------                         */
/*                   Module: elec_energy_control.c                          */
/*                                                                          */
/* This routine calls the required force and PE routines                    */
/*                                                                          */
/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/

#include "standard_include.h"
#include "../typ_defs/typedefs_gen.h"
#include "../typ_defs/typedefs_class.h"
#include "../typ_defs/typedefs_bnd.h"
#include "../typ_defs/typedefs_cp.h"
#include "../proto_defs/proto_energy_cp_local.h"
#include "../proto_defs/proto_energy_cpcon_local.h"
#include "../proto_defs/proto_friend_lib_entry.h"
#include "../proto_defs/proto_math.h"
#include "../proto_defs/proto_communicate_wrappers.h"

#define TIME_CP_OFF

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/

void cp_ks_energy_dvr_full_g(CP *cp,int ip_now,EWALD *ewald,EWD_SCR *ewd_scr,
                             CELL *cell,CLATOMS_INFO *clatoms_info,
                             CLATOMS_POS *clatoms_pos,ATOMMAPS *atommaps,
                             STAT_AVG *stat_avg,PTENS *ptens,SIMOPTS *simopts,
                             FOR_SCR *for_scr)


/*==========================================================================*/
/*         Begin Routine                                                    */
{/*Begin Routine*/
/*=======================================================================*/
/*         Local Variable declarations                                   */

#include "../typ_defs/typ_mask.h"

  int i;
  int cp_debug_xc=0;
  int cp_min_on;

  static int itime=0,itime_nl=0;
  double cpu1,cpu2,cpusum;

/* Local pointers */

  int cp_wave_min       = simopts->cp_wave_min;
  int cp_min            = simopts->cp_min;
  int cp_wave_min_pimd  = simopts->cp_wave_min_pimd;

  int cp_lsda           = cp->cpopts.cp_lsda;
  int cp_nonint         = cp->cpopts.cp_nonint;
  int cp_norb           = cp->cpopts.cp_norb;
  int cp_gga            = cp->cpopts.cp_gga;
  int cp_ke_dens_on     = cp->cpcoeffs_info.cp_ke_dens_on;
  int cp_elf_calc_frq   = cp->cpcoeffs_info.cp_elf_calc_frq;

  int np_states         = cp->communicate.np_states;
  int myid_state        = cp->communicate.myid_state;
  MPI_Comm  comm_states = cp->communicate.comm_states;

  char *ggax_typ        = cp->pseudo.ggax_typ;
  char *ggac_typ        = cp->pseudo.ggac_typ;

  int nstate_up_proc = cp->cpcoeffs_info.nstate_up_proc;
  int nstate_up      = cp->cpcoeffs_info.nstate_up;
  int *ioff_upt      = cp->cpcoeffs_info.ioff_upt;
  double scale_fact  = cp->cpcoeffs_info.scale_fact;
  double *occ_up     = cp->cpopts.occ_up;

  int nstate_dn_proc = cp->cpcoeffs_info.nstate_dn_proc;
  int nstate_dn      = cp->cpcoeffs_info.nstate_dn;
  int *ioff_dnt      = cp->cpcoeffs_info.ioff_dnt;
  double *occ_dn     = cp->cpopts.occ_dn;

  int *icoef_orth_up    = &(cp->cpcoeffs_pos_dvr[ip_now].icoef_orth_up);
  int *icoef_form_up    = &(cp->cpcoeffs_pos_dvr[ip_now].icoef_form_up);
  int *ifcoef_orth_up   = &(cp->cpcoeffs_pos_dvr[ip_now].ifcoef_orth_up);
  int *ifcoef_form_up   = &(cp->cpcoeffs_pos_dvr[ip_now].ifcoef_form_up);

  double *dvrc_up       = cp->cpcoeffs_pos_dvr[ip_now].dvrc_up;
  double *dvrfc_up      = cp->cpcoeffs_pos_dvr[ip_now].dvrfc_up;
  double *fcre_up       = cp->cpscr.cpscr_wave.cre_up;
  double *fcim_up       = cp->cpscr.cpscr_wave.cim_up;


  int *icoef_orth_dn    = &(cp->cpcoeffs_pos_dvr[ip_now].icoef_orth_dn);
  int *icoef_form_dn    = &(cp->cpcoeffs_pos_dvr[ip_now].icoef_form_dn);
  int *ifcoef_orth_dn   = &(cp->cpcoeffs_pos_dvr[ip_now].ifcoef_orth_dn);
  int *ifcoef_form_dn   = &(cp->cpcoeffs_pos_dvr[ip_now].ifcoef_form_dn);

  double *dvrc_dn       = cp->cpcoeffs_pos_dvr[ip_now].dvrc_dn;
  double *dvrfc_dn      = cp->cpcoeffs_pos_dvr[ip_now].dvrfc_dn;
  /* this looks odd. make it consisten with up states  
  double *fcre_dn       = cp->cpcoeffs_pos_dvr[ip_now].fcre_dn;
  double *fcim_dn       = cp->cpcoeffs_pos_dvr[ip_now].fcim_dn;
  */
  double *fcre_dn       = cp->cpscr.cpscr_wave.cre_dn;
  double *fcim_dn       = cp->cpscr.cpscr_wave.cim_dn;


  double *ksmat_up       = cp->cpcoeffs_pos_dvr[ip_now].ksmat_up;
  double *norbmat_up     = cp->cpcoeffs_pos_dvr[ip_now].norbmat_up;
  double *norbmati_up    = cp->cpcoeffs_pos_dvr[ip_now].norbmati_up;
  double *ovmat_eigv_up  = cp->cpcoeffs_pos_dvr[ip_now].ovmat_eigv_up;
  double *ksmat_dn       = cp->cpcoeffs_pos_dvr[ip_now].ksmat_dn;
  double *norbmat_dn     = cp->cpcoeffs_pos_dvr[ip_now].norbmat_dn;
  double *norbmati_dn    = cp->cpcoeffs_pos_dvr[ip_now].norbmati_dn;
  double *ovmat_eigv_dn  = cp->cpcoeffs_pos_dvr[ip_now].ovmat_eigv_dn;

  double *max_diag     = &(cp->cpcoeffs_pos[ip_now].max_diag);
  double *max_off_diag = &(cp->cpcoeffs_pos[ip_now].max_off_diag);

  double *cpscr_cre_up   = cp->cpscr.cpscr_wave.cre_up;
  double *rho_up         = cp->cpscr.cpscr_rho.rho_up;
  double *rhocr_up       = cp->cpscr.cpscr_rho.rhocr_up;
  double *rhoci_up       = cp->cpscr.cpscr_rho.rhoci_up;

  double *zfft           = cp->cpscr.cpscr_wave.zfft;
  double *zfft_tmp       = cp->cpscr.cpscr_wave.zfft_tmp;

  double *cpscr_cre_dn   = cp->cpscr.cpscr_wave.cre_dn;
  double *rho_dn         = cp->cpscr.cpscr_rho.rho_dn;
  double *rhoci_dn       = cp->cpscr.cpscr_rho.rhoci_dn;
  double *rhocr_dn       = cp->cpscr.cpscr_rho.rhocr_dn;

  double *ksmat_scr      = cp->cpscr.cpscr_ovmat.ovlap1;

  int   nfft             = cp->cp_para_fft_pkg3d_sm.nfft;
  int   nfft_proc        = cp->cp_para_fft_pkg3d_sm.nfft_proc;
  int   nfft2_proc       = nfft_proc/2;

  int   ncoef_l_use      = nfft/2;

  double *x              = clatoms_pos->x;
  double *y              = clatoms_pos->y;
  double *z              = clatoms_pos->z;
  double *fx             = clatoms_pos->fx;
  double *fy             = clatoms_pos->fy;
  double *fz             = clatoms_pos->fz;
  int iatm;

  int nkf1  = cp->cp_para_fft_pkg3d_sm.nkf1;
  int nkf2  = cp->cp_para_fft_pkg3d_sm.nkf2;
  int nkf3  = cp->cp_para_fft_pkg3d_sm.nkf3;

  int *iatm_atm_typ       = atommaps->iatm_atm_typ;
  int natm_tot            = clatoms_info->natm_tot;
  int iperd               = cell->iperd;
  double *hmat            = cell->hmat;
  double tol_edge_dist    = cp->cpopts.tol_edge_dist;
  int icheck_perd_size    = cp->cpopts.icheck_perd_size;
  int cp_dual_grid_opt_on = cp->cpopts.cp_dual_grid_opt;

  double *ptens_pvten_tot = ptens->pvten_tot;
  double *ptens_pvten     = ptens->pvten;
  double *ptens_pvten_tmp = ptens->pvten_tmp;

  int nloc_trunc_on = cp->cpopts.cp_nl_trunc_opt;
  int nloc_wan_on = cp->cpopts.cp_nloc_wan_opt;
  int nmax_wan_orb = cp->cp_wannier.nmax_wan_orb;
  double **wan_cent = cp->electronic_properties.wannier_cent;
  double r_cut_wan = cp->cp_wannier.rcut_wan_nl;

  int j,k,icoef_form_tmp;
  double cp_eke;

/*======================================================================*/
/* 0.1) Check the forms                                                   */
/*  What has not yet been implemented for DVR */

  if(cp_ke_dens_on==1){
    printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
    printf("CP_KE_DENS HAS NOT BEEN IMPLEMENTED FOR THE DVR BASIS SET \n");
    printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
    fflush(stdout);
    exit(1);
  }/*endif*/

  if(cp_elf_calc_frq > 0 && ip_now==1){
   printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
   printf("CP_ELF_CALC_FRQ HAS NOT BEEN IMPLEMENTED FOR THE DVR BASIS SET \n");
   printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
   fflush(stdout);
   exit(1);
  }/*endif*/

  if(cp_norb>0){
    if((*icoef_orth_up)!=0){
      printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
      printf("Up Coefs must be in nonorthonormal form under norb \n");
      printf("on state processor %d in cp_elec_energy_ctrl \n",myid_state);
      printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
      fflush(stdout);
      exit(1);
    }/*endif*/
    if(cp_lsda==1){
      if((*icoef_orth_dn)!=0){
        printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
        printf("Dn Coefs must be in nonorthonormal form under norb \n");
        printf("on state processor %d in cp_elec_energy_ctrl \n",myid_state);
        printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
        fflush(stdout);
        exit(1);
      }/*endif*/
    }/*endif*/
  }/*endif*/


/*======================================================================*/
/* 0.2) Check the approximations in the methods                        */

  /* probably don't need this for DVR/cluster 

  if( ((iperd<3) || (iperd==4)) && (icheck_perd_size==1) ){
    cp_boundary_check(cell,clatoms_info,clatoms_pos,tol_edge_dist);
  }
  */
  

/*======================================================================*/
/* 0.3) Set the GGA flags                                               */

  cp->cpopts.cp_becke    =0;
  cp->cpopts.cp_b3       =0;
  cp->cpopts.cp_pw91x    =0;
  cp->cpopts.cp_fila_1x  =0;
  cp->cpopts.cp_fila_2x  =0;
  cp->cpopts.cp_pbe_x    =0;
  cp->cpopts.cp_brx89    =0;
  cp->cpopts.cp_brx2k    =0;
  cp->cpopts.cp_lyp      =0;
  cp->cpopts.cp_lypm1    =0;
  cp->cpopts.cp_pw91c    =0;
  cp->cpopts.cp_pbe_c    =0;
  cp->cpopts.cp_tau1_c   =0;
  cp->cpopts.cp_debug_xc =0;

  if(cp_gga == 1){
    if(strcasecmp(ggax_typ,"becke"   )==0){cp->cpopts.cp_becke=1;}
    if(strcasecmp(ggax_typ,"B3"      )==0){cp->cpopts.cp_b3=1;}
    if(strcasecmp(ggax_typ,"pw91x"   )==0){cp->cpopts.cp_pw91x=1;}
    if(strcasecmp(ggax_typ,"fila_1x" )==0){cp->cpopts.cp_fila_1x=1;}
    if(strcasecmp(ggax_typ,"fila_2x" )==0){cp->cpopts.cp_fila_2x=1;}
    if(strcasecmp(ggax_typ,"pbe_x"   )==0){cp->cpopts.cp_pbe_x=1;}
    if(strcasecmp(ggax_typ,"brx89"   )==0){cp->cpopts.cp_brx89=1;}
    if(strcasecmp(ggax_typ,"brx2k"   )==0){cp->cpopts.cp_brx2k=1;}
    if(strcasecmp(ggac_typ,"lyp"     )==0){cp->cpopts.cp_lyp=1;  }
    if(strcasecmp(ggac_typ,"lypm1"   )==0){cp->cpopts.cp_lypm1=1;}
    if(strcasecmp(ggac_typ,"pw91c"   )==0){cp->cpopts.cp_pw91c=1;}
    if(strcasecmp(ggac_typ,"pbe_c"   )==0){cp->cpopts.cp_pbe_c=1;}
    if(strcasecmp(ggac_typ,"tau1_c"  )==0){cp->cpopts.cp_tau1_c=1;}
    if(strcasecmp(ggac_typ,"debug97x")==0){cp->cpopts.cp_debug_xc=1;}
  }/*endif*/

/*======================================================================*/
/* I) Orthogonalize the coefs if norbing                                */

  if(cp_norb>0){
    (*max_diag)     = 0.0;
    (*max_off_diag) = 0.0;

    cp_rotate_coef_ortho_dvr(dvrc_up,*icoef_form_up,icoef_orth_up,
                             norbmat_up,norbmati_up,ovmat_eigv_up,
                             cpscr_cre_up, occ_up,ioff_upt,max_off_diag,
                             max_diag, scale_fact, &(cp->cpscr.cpscr_ovmat),
                             &(cp->cp_comm_state_pkg_dvr_up));

    if((cp_lsda==1) && (nstate_dn_proc > 0) ){
      cp_rotate_coef_ortho_dvr(dvrc_dn,*icoef_form_dn,icoef_orth_dn,
                               norbmat_dn,norbmati_dn,ovmat_eigv_dn,
                               cpscr_cre_dn, occ_dn,ioff_dnt,max_off_diag,
                               max_diag, scale_fact, &(cp->cpscr.cpscr_ovmat),
                               &(cp->cp_comm_state_pkg_dvr_dn));

    }/*endif*/
  }/*endif*/

/*======================================================================*/
/* II) Get the total density, (spin densities too if lsda)              */

#ifdef TIME_CP
  if(np_states>1){Barrier(comm_states);} cputime(&cpu1);
#endif

  cp_rho_calc_dvr_all_space(&(cp->cpscr),cell,dvrc_up,*icoef_form_up,
                            *icoef_orth_up, rhocr_up,rhoci_up,rho_up,
                            nstate_up,nfft,nfft2_proc,nkf1,nkf2,nkf3,
                            &(cp->communicate),&(cp->cp_para_fft_pkg3d_sm));

  if((cp_lsda == 1) && (nstate_dn != 0) ){
    cp_rho_calc_dvr_all_space(&(cp->cpscr),cell, dvrc_dn,*icoef_form_dn,
                              *icoef_orth_dn, rhocr_dn,rhoci_dn,rho_dn,
                              nstate_dn,nfft,nfft2_proc,nkf1,nkf2,nkf3,
                              &(cp->communicate),&(cp->cp_para_fft_pkg3d_sm));

    /* total density in g-space */ 
    for(i=1;i <= nfft2_proc;i++) {
      rhocr_up[i] += rhocr_dn[i];
      rhoci_up[i] += rhoci_dn[i];
    }/* endfor */
  }/* endif */


#ifdef TIME_CP
   cputime(&cpu2);  par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,
                     "cp_rho_calc_dvr_full_g");
#endif

/*======================================================================*/
/* III) Initialize forces, pressure tensor, inverse hmat                  */

  (*ifcoef_form_up) = 1;  /* transpose form nstate x nfft_proc */
  (*ifcoef_orth_up) = 1;

  for(i=1;i<= nfft2_proc*nstate_up;i++){
    dvrfc_up[i] = 0.0; 
  }
  for(i=1;i<= nstate_up_proc*ncoef_l_use;i++){
    fcre_up[i] = 0.0;
    fcim_up[i] = 0.0;
  }

  if( (cp_lsda == 1) && (nstate_dn != 0)){
    (*ifcoef_form_dn) = 1;
    (*ifcoef_orth_dn) = 1;
    for(i=1;i<=nfft2_proc*nstate_dn;i++){dvrfc_dn[i] = 0.0;}
    for(i=1;i<= nstate_dn_proc*ncoef_l_use;i++){
      fcre_dn[i] = 0.0;
      fcim_dn[i] = 0.0;
    }
  }

  for(i=1;i<=9;i++){ptens_pvten_tmp[i] = 0.0;}

  for(i=1;i<=natm_tot;i++){
    fx[i]=0.0;
    fy[i]=0.0;
    fz[i]=0.0;
  }

  /* probably, iper does not matter if the cell is cubic */
  gethinv(cell->hmat_cp,cell->hmati_cp,&(cell->vol_cp),iperd);
  gethinv(cell->hmat,cell->hmati,&(cell->vol),iperd);

/*======================================================================*/
/* IV) Calculate the local energy */

#ifdef TIME_CP
  if(np_states>1){Barrier(comm_states);} cputime(&cpu1);
#endif

  if(iperd==0){

    eext_loc_real_space_dvr(clatoms_pos,atommaps,&(cp->pseudo),ewd_scr,cell,
                            &(cp->communicate),
                            rho_up,dvrc_up,dvrfc_up,natm_tot,nstate_up,
                            &(stat_avg->cp_eext),&(cp->cp_para_fft_pkg3d_sm));

    if(cp_lsda==1){
      eext_loc_real_space_dvr(clatoms_pos,atommaps,&(cp->pseudo),ewd_scr,cell,
                              &(cp->communicate),
                              rho_dn,dvrc_dn,dvrfc_dn,natm_tot,nstate_dn,
                              &(stat_avg->cp_eext),&(cp->cp_para_fft_pkg3d_sm));
    }

  }else if(iperd ==3){

    /*compute E_recip and E_loc */
    /* fcre, fcim are NOT scartch in 3D. They contain Vextr, Vexti info */

    eext_loc_recip_space_dvr(clatoms_info,clatoms_pos,cell,ewald,
                             &(cp->cpewald),&(cp->cpscr),&(cp->pseudo),
                             ewd_scr,atommaps,fcre_up,fcim_up,
                             &(stat_avg->vrecip),&(stat_avg->cp_eext),
                             &(cp->communicate),for_scr, nkf1,nkf2,nkf3);
    //printf("myid_state  %d,cp_eext %f 1\n", myid_state,stat_avg->cp_eext);

    if(cp_lsda==1){
      eext_loc_recip_space_dvr(clatoms_info,clatoms_pos,cell,ewald,
                               &(cp->cpewald),&(cp->cpscr),&(cp->pseudo),
                               ewd_scr,atommaps,fcre_dn,fcim_dn,
                               &(stat_avg->vrecip),&(stat_avg->cp_eext),
                               &(cp->communicate),for_scr, nkf1,nkf2,nkf3);
    }
    // printf("myid_state  %d,cp_eext %f 2\n", myid_state,stat_avg->cp_eext);

  }else{
    if(myid_state==0){
      printf("@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
      printf(" WRONG CHOICE FOR BOUNDARY CONDITION in cp_ks_energy_dvr  \n");
      printf("@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
    }
    fflush(stdout);
    exit(1);
  }

#ifdef TIME_CP
  cputime(&cpu2);
  par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,
                     "local potential");
#endif

/*===============================================================================*/
/* V) Hartree term  */

#ifdef TIME_CP
  if(np_states>1){Barrier(comm_states);} cputime(&cpu1);
#endif

  if(iperd == 0){

    calc_hartree_dvr_clus(&(cp->cpcoeffs_info),&(cp->cpscr.cpscr_wave),
                          &(cp->cp_dvr_clus),rho_up,fcre_up, fcim_up,
                          dvrfc_up, dvrc_up, &(stat_avg->cp_ehart),
                          nfft2_proc, nstate_up,&(cp->communicate));

  }else if(iperd == 3){

    calc_hartree_dvr_pbc(&(cp->cpopts),&(cp->cpewald),rhocr_up,rhoci_up,
                         zfft,zfft_tmp,fcre_up,fcim_up,dvrfc_up,dvrc_up,
                         dvrfc_dn,dvrc_dn,&(stat_avg->cp_ehart),
                         nstate_up,nstate_dn, ewald,cell,
                         &(cp->communicate), nkf1,nkf2,nkf3,nfft,nfft_proc,
                         cp->cpscr.cpscr_wave.fft_map,
                         cp->cpscr.cpscr_wave.fft_map_rev,
                         &(cp->cp_para_fft_pkg3d_sm));

    //printf("myid_state  %d, %f cp_ehart 1\n", myid_state,stat_avg->cp_ehart);

  }

#ifdef TIME_CP
  cputime(&cpu2);
  par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,"hartree_dvr");
#endif

/*======================================================================*/
/*  VI) Calculate the non-local pseudopotential list                    */

  if(itime == 0 ){

    /* Make it sure that no cp_lsda for Wannier */

    cp->cpscr.cpscr_nonloc.nloc_trunc_on=nloc_trunc_on;
    cp->cpscr.cpscr_nonloc.nloc_wan_on=nloc_wan_on;
    cp->cpscr.cpscr_nonloc.nmax_wan_orb = nmax_wan_orb;

    if(nloc_trunc_on==1 && nloc_wan_on==0){
      if(nmax_wan_orb != nstate_up){
        if(myid_state==0){
          printf("$$$$$$$$$$$$$$$$$$$$$$$$_WARNING_$$$$$$$$$$$$$$$$$$$$$$$\n");
          printf("  You choose nonlocal grid truncation option, but not   \n");
          printf(" the Wannier truncation. In such case, nmax_wan_orb     \n");
          printf(" should be equal to the number of state. We will correct\n");
          printf(" this error for you and set nmax_wan_orb=nstate_up      \n");
          printf("$$$$$$$$$$$$$$$$$$$$$$$$_WARNING_$$$$$$$$$$$$$$$$$$$$$$$\n");
        }
        cp->cpscr.cpscr_nonloc.nmax_wan_orb=nstate_up;
      }
    }

    vps_atm_list_dvr(&(cp->pseudo),cell, &(cp->cp_para_fft_pkg3d_sm),
                     &(cp->cpscr.cpscr_nonloc),atommaps,natm_tot,nstate_up);

  }/*endif first time*/

#ifdef TIME_CP
  if(np_states>1){Barrier(comm_states);} cputime(&cpu1);
#endif

  if(cp->pseudo.np_nonloc_cp_box > 0){

    itime_nl+=1;
    control_eext_nonloc_dvr(atommaps,&(cp->pseudo),clatoms_pos,
                            &(cp->cpscr.cpscr_nonloc),
                            &(cp->cpscr.cpscr_wave),ewd_scr,
                            &(cp->cpopts),&(cp->cpcoeffs_info),
                            &(cp->cpcoeffs_pos_dvr[ip_now]),
                            cell,&(cp->communicate),
                            &(cp->cp_para_fft_pkg3d_sm),
                            natm_tot,&(stat_avg->cp_enl),
                            itime_nl,wan_cent,r_cut_wan);

  }

#ifdef TIME_CP
  cputime(&cpu2);  par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,
                     "non-local");
#endif


/*========================================================================*/
/* VII) Calculate the exchange and correlation energy and contribution to */
/*      coefficient forces */

  if(cp->cpopts.cp_nonint == 0){
    stat_avg->cp_exc  = 0.00;
    stat_avg->cp_muxc = 0.00;

    if(np_states>1){Barrier(comm_states);}

#ifdef TIME_CP
    cputime(&cpu1);
#endif

    control_exc_real_space_dvr (cell,ewald,&(cp->pseudo),&(cp->cpopts),
                            &(cp->cpcoeffs_info),&(cp->cpscr.cpscr_wave),
                            &(cp->cp_para_fft_pkg3d_sm),
                            &(cp->cpcoeffs_pos_dvr[ip_now]),
                            &(cp->communicate), &(cp->cpscr),
                            &(cp->dvr_matrix), zfft, zfft_tmp,
                            &(cp->cp_comm_state_pkg_dvr_up),
                            &(cp->cp_comm_state_pkg_dvr_dn),
                            &(stat_avg->cp_exc), &(stat_avg->cp_muxc));

#ifdef TIME_CP
      cputime(&cpu2); par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,
                      "exchange-correlation");
#endif
  }/*endif e-e interact*/

/*======================================================================*/
/* VIII) Calculate the electronic kinetic energy                          */

#ifdef TIME_CP
  if(np_states>1){Barrier(comm_states);}
  cputime(&cpu1);
#endif

  if (np_states > 1 && cp_gga==0){
    Barrier(comm_states);
    icoef_form_tmp=1;
    cp_transpose_bck_dvr(dvrc_up,&(icoef_form_tmp), cpscr_cre_up,
                          &(cp->cp_comm_state_pkg_dvr_up), nkf1,nkf2,nkf3);
    icoef_form_tmp=1;
    cp_transpose_bck_dvr(dvrfc_up,&(icoef_form_tmp), cpscr_cre_up,
                          &(cp->cp_comm_state_pkg_dvr_up), nkf1,nkf2,nkf3);
  }

  calc_ekin_dvr_par(dvrc_up,dvrfc_up,&(cp->dvr_matrix),
                    &(cp->cpcoeffs_info),nstate_up,&cp_eke,
                    (cp->cpscr.cpscr_wave.zfft),
                    (cp->cpscr.cpscr_wave.zfft_tmp),cell->vol_cp,
                    &(cp->communicate),
                    &(cp->cp_para_fft_pkg3d_sm));

  stat_avg->cp_eke = cp_eke;

  if(np_states > 1){

    Barrier(comm_states);
    icoef_form_tmp=0;
    cp_transpose_fwd_dvr(dvrc_up,&(icoef_form_tmp), cpscr_cre_up,
                         &(cp->cp_comm_state_pkg_dvr_up), nkf1,nkf2,nkf3);
    icoef_form_tmp=0;
    cp_transpose_fwd_dvr(dvrfc_up,&(icoef_form_tmp), cpscr_cre_up,
                         &(cp->cp_comm_state_pkg_dvr_up), nkf1,nkf2,nkf3);
  }


  if(cp_lsda){
    if (np_states > 1 && cp_gga==0){

      Barrier(comm_states);
      icoef_form_tmp=1;
      cp_transpose_bck_dvr(dvrc_dn,&(icoef_form_tmp), cpscr_cre_dn,
                           &(cp->cp_comm_state_pkg_dvr_dn), nkf1,nkf2,nkf3);
      icoef_form_tmp=1;
      cp_transpose_bck_dvr(dvrfc_up,&(icoef_form_tmp), cpscr_cre_dn,
                           &(cp->cp_comm_state_pkg_dvr_dn), nkf1,nkf2,nkf3);
    }

    calc_ekin_dvr_par(dvrc_dn,dvrfc_dn,&(cp->dvr_matrix),
                      &(cp->cpcoeffs_info),nstate_up,&cp_eke,
                      (cp->cpscr.cpscr_wave.zfft),
                      (cp->cpscr.cpscr_wave.zfft_tmp),cell->vol_cp,
                      &(cp->communicate),
                      &(cp->cp_para_fft_pkg3d_sm));

    stat_avg->cp_eke += cp_eke;

    if(np_states > 1){

      Barrier(comm_states);
      icoef_form_tmp=0;
      cp_transpose_fwd_dvr(dvrc_dn,&(icoef_form_tmp), cpscr_cre_dn,
                           &(cp->cp_comm_state_pkg_dvr_dn), nkf1,nkf2,nkf3);

      icoef_form_tmp=0;
      cp_transpose_fwd_dvr(dvrfc_dn,&(icoef_form_tmp), cpscr_cre_dn,
                           &(cp->cp_comm_state_pkg_dvr_dn), nkf1,nkf2,nkf3);
    }
  }/*endif cp_lsda*/

#ifdef TIME_CP
  cputime(&cpu2);
  par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,
                     "Kinetic Energy ");
#endif

/*======================================================================*/
/* IX) KS matrix used as an approximation  to the Lagrange multipliers  */
/*   when minimizing. It is a necessary part of the forces when norbing */

  cp_min_on = cp_wave_min + cp_min + cp_wave_min_pimd;

  if( (cp_min_on+cp_norb) >= 1 ){

#ifdef TIME_CP
    if(np_states>1){Barrier(comm_states);}
    cputime(&cpu1);
#endif

    cp_add_ksmat_force_dvr(dvrc_up,*icoef_form_up,*icoef_orth_up,
                           dvrfc_up,*ifcoef_form_up,*ifcoef_orth_up,
                           ksmat_up,ksmat_scr,ioff_upt,cp_lsda,
                           cp_min_on,occ_up,cp->cpcoeffs_info.scale_fact,
                           &(cp->cp_comm_state_pkg_dvr_up));

    if( (cp_lsda==1) && (nstate_dn !=0) ){
      cp_add_ksmat_force_dvr(dvrc_dn,*icoef_form_dn,*icoef_orth_dn,
                             dvrfc_dn,*ifcoef_form_dn,*ifcoef_orth_dn,
                             ksmat_dn,ksmat_scr,ioff_dnt,cp_lsda,
                             cp_min_on,occ_dn,
                             cp->cpcoeffs_info.scale_fact,
                             &(cp->cp_comm_state_pkg_dvr_dn));
    }/*endif*/

#ifdef TIME_CP
    cputime(&cpu2);
    par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,
                     "cp_add_ksmat_force");
#endif
  }/*endif*/

/*======================================================================*/
/* X) DeOrthogonalize the coefs and coef forces for norb:       */
/*    This requires  a matrix and its inverse (chain rule)      */

  if(cp_norb>0){
    if(np_states>1){Barrier(comm_states);}
#ifdef TIME_CP
    cputime(&cpu1);
#endif
    cp_rotate_gen_nonortho_dvr(dvrc_up,*icoef_form_up,icoef_orth_up,
                               norbmati_up,ioff_upt,cpscr_cre_up,
                               &(cp->cp_comm_state_pkg_dvr_up));
    cp_rotate_gen_nonortho_dvr(dvrfc_up,*ifcoef_form_up,ifcoef_orth_up,
                               norbmat_up,ioff_upt,cpscr_cre_up,
                              &(cp->cp_comm_state_pkg_dvr_up));
    if((cp_lsda==1) && (nstate_dn > 0) ){
      cp_rotate_gen_nonortho_dvr(dvrc_dn,*icoef_form_dn,icoef_orth_dn,
                                 norbmati_dn,ioff_dnt,cpscr_cre_dn,
                                 &(cp->cp_comm_state_pkg_dvr_dn));
      cp_rotate_gen_nonortho_dvr(dvrfc_dn,*ifcoef_form_dn,ifcoef_orth_dn,
                                 norbmat_dn,ioff_dnt,cpscr_cre_dn,
                                 &(cp->cp_comm_state_pkg_dvr_dn));
    }/*endif:lsda*/

#ifdef TIME_CP
    cputime(&cpu2);
    par_cpu_vomit((cpu2-cpu1),comm_states,np_states,myid_state,
                     "cp_rotate_get_nonortho");
#endif
  }/*endif:cp_norb*/

/*======================================================================*/
/* XI) Increment the pressure tensor                                    */

  for(i=1;i<=9;i++){
    ptens_pvten_tot[i] += ptens_pvten_tmp[i];
    ptens_pvten[i]     += ptens_pvten_tmp[i];
  }/*endfor*/


  itime=1;

/*==========================================================================*/
}/*end routine*/
/*==========================================================================*/


/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*==========================================================================*/

void cp_rho_calc_dvr_all_space(CPSCR *cpscr,CELL *cell,
                               double *dvrc,int icoef_form,int icoef_orth,
                               double *rhocr,double *rhoci,double *rho,
                               int nstate,int nfft,int nfft2_proc,
                               int nkf1,int nkf2,int nkf3,
                               COMMUNICATE *communicate,
                               PARA_FFT_PKG3D *cp_para_fft_pkg3d_sm)

/*==========================================================================*/
{/*begin routine*/
/*==========================================================================*/
#include "../typ_defs/typ_mask.h"

/*    local variables       */

 int iii,icount,ioff,index,is,i,j,k;
 int ip,jp,kp;
 double vol_cp;

/*    Assign local pointers     */

 double *zfft           =    cpscr->cpscr_wave.zfft;
 double *zfft_tmp       =    cpscr->cpscr_wave.zfft_tmp;
 double *hmati_cp       =    cell->hmati_cp;
 double *hmat_cp        =    cell->hmat_cp;

 int *ifft              =    cpscr->cpscr_wave.ifft;
 int *jfft              =    cpscr->cpscr_wave.jfft;
 int *kfft              =    cpscr->cpscr_wave.kfft;

 int   nfft2            =    nfft/2;
 int   nktot            =    nfft2 - 1; /* number of non-zero g-vectors */

 int   myid_state       =    communicate->myid_state;
 int   np_states        =    communicate->np_states;
 MPI_Comm comm_states   =    communicate->comm_states;

 int *irecv,*idispl;
 double *rhocr_scr   =  cpscr->cpscr_wave.zfft_tmp;       
 double *rhoci_scr   = &cpscr->cpscr_wave.zfft_tmp[nfft2];
 double *rhocr_tmp   =  cpscr->cpscr_wave.zfft;          
 double *rhoci_tmp   = &cpscr->cpscr_wave.zfft[nfft2];   

 int *fft_map           =    cpscr->cpscr_wave.fft_map;
 int idiv,irem;

/*------------------------------------------------------------------------------- */
/* 0) Check the form of the coefficients  */

  if(icoef_orth!=1){
    printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
    printf("The coefficients must be in orthogonal form    \n");
    printf("on state processor %d in cp_rho_calc_dvr_all_space \n",myid_state);
    printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
    fflush(stdout);exit(1);
  }/*endif*/

  if(np_states>1 && icoef_form == 0){
    printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
    printf("The coefficients must be in transposed (not normal) \n");
    printf("form on state processor %d in cp_rho_calc_dvr_all_sace \n",myid_state);
    printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
    fflush(stdout);exit(1);
  }/*endif*/

/*---------------------------------------------------------------------------*/
/* 1) zero density */

  irecv  = (int *) malloc((np_states)*sizeof(int))-1;
  idispl = (int *) malloc((np_states)*sizeof(int))-1;

  for(i=1; i<= nfft2_proc ; i++){ 
    rho[i] = 0.0; 
  }

  vol_cp  = getdeth(hmat_cp);

/*--------------------------------------------------------------------------*/
/* 2)sum the density in real space */

  for(is=1;is <= nstate;is++){
    ioff  = (is-1)*nfft2_proc;
    for(i=1; i<= nfft2_proc ; i++){
      rho[i] += dvrc[(ioff+i)]*dvrc[(ioff+i)];
    }
  }

/*--------------------------------------------------------------------------*/
/* 3) Gather Density on Proc 0:  Only Proc 0 Performs FFT */

  if( np_states > 1){

    idiv    = (nkf2*nkf3)/np_states;
    irem    = (nkf2*nkf3)%np_states;

    for(i=0; i <irem; i++){ irecv[i+1] = (idiv+1)*nkf1;  }
    for(i=irem ; i < np_states; i++){ irecv[i+1] = idiv*nkf1;  }

    idispl[1] = 0;
    for(i=2; i <= np_states; i++){
      idispl[i] = idispl[i-1] + irecv[i-1];
    }

    Barrier(comm_states);
    Gatherv(&rho[1],nfft2_proc,MPI_DOUBLE,&zfft_tmp[1],
            &irecv[1],&idispl[1],MPI_DOUBLE,0,comm_states);
  }else{
    for(i=1; i <= nfft2; i++){ zfft_tmp[i] = rho[i]; }
  }

/*---------------------------------------------------------------------------*/
/* 4) SERIAL : FFT the density to g-space */

  if(myid_state == 0){
    /* preparation for FFT */
    icount = 0;
    for(k=1; k <= nkf3; k++){
      kp = kfft[k];
      for(j=1; j <= nkf2; j++){
        jp = jfft[j];
        for(i=1; i <= nkf1; i++){
          ip = ifft[i];
          iii = kp*nkf1*nkf2 + jp*nkf1 + ip ;
          icount++;
          zfft[(2*iii+1)] =  zfft_tmp[icount];
          zfft[(2*iii+2)] =  0.00;
    }}} /*endfor*/
 
    /* serial FFT to g-space */
    para_fft_gen3d_dvr(zfft,zfft_tmp,cp_para_fft_pkg3d_sm,1);    

    /* Unpacking with volume factor*/
    for(i=0; i < (nkf1*nkf2*nkf3); i++){
      rhocr_scr[i+1] = zfft[(2*i+1)]*vol_cp;
      rhoci_scr[i+1] = zfft[(2*i+2)]*vol_cp;
    }
    for(i=1; i <= nfft2; i++){
      index = fft_map[i];
      rhocr_tmp[i] = rhocr_scr[index];
      rhoci_tmp[i] = rhoci_scr[index];
    }

  }/*end myid: done with FFT */
 
/*--------------------------------------------------------------------------*/
/* 5) Broadcast the g-space density : full_g style */

  if(np_states > 1){
    Barrier(comm_states);
    Scatterv(&rhocr_tmp[1],&irecv[1],&idispl[1],MPI_DOUBLE,
             &rhocr[1],nfft2_proc,MPI_DOUBLE,0,comm_states);

    Barrier(comm_states);
    Scatterv(&rhoci_tmp[1],&irecv[1],&idispl[1],MPI_DOUBLE,
             &rhoci[1],nfft2_proc,MPI_DOUBLE,0,comm_states);
  }else{
    for(i=1; i <= nfft2; i++){
      rhocr[i] = rhocr_tmp[i];
      rhoci[i] = rhoci_tmp[i];
    }
  }

/*--------------------------------------------------------------*/
/* 6) Free locally assigned memory */

  free(&irecv[1]);
  free(&idispl[1]);

/*==============================================================*/
}/*end routine*/
/*==============================================================*/

