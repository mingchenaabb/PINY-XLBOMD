/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*                                                                          */
/*                         PI_MD:                                           */
/*             The future of simulation technology                          */
/*             ------------------------------------                         */
/*                     Module: int_NVE                                      */
/*                                                                          */
/* This subprogram integrates the system using Vel Verlet                   */
/*                                                                          */
/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/


#include "standard_include.h"
#include "../typ_defs/typedefs_gen.h"
#include "../typ_defs/typedefs_class.h"
#include "../typ_defs/typedefs_bnd.h"
#include "../typ_defs/typedefs_cp.h"
#include "../proto_defs/proto_integrate_cpmin_entry.h"
#include "../proto_defs/proto_integrate_cpmin_local.h"
#include "../proto_defs/proto_energy_cpcon_entry.h"
#include "../proto_defs/proto_energy_cpcon_local.h"
#include "../proto_defs/proto_intra_con_entry.h"
#include "../proto_defs/proto_energy_ctrl_cp_entry.h"
#include "../proto_defs/proto_energy_ctrl_local.h"
#include "../proto_defs/proto_energy_cp_local.h"
#include "../proto_defs/proto_friend_lib_entry.h"
#include "../proto_defs/proto_communicate_wrappers.h"
#include "../proto_defs/proto_math.h"

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
void min_CG_cp_dvr(CLASS *class,BONDED *bonded,GENERAL_DATA *general_data,
                   CP *cp, int ip_now)
/*========================================================================*/
{/*begin routine*/
/*========================================================================*/
/*             Local variable declarations                                */
#include "../typ_defs/typ_mask.h"

    /* local pointers */

    int np_states        = class->communicate.np_states;
    MPI_Comm comm_states = class->communicate.comm_states;
    int myid_state       = cp->communicate.myid_state;

    double *ptens_pvten        = general_data->ptens.pvten;
    double *ptens_pvten_tot    = general_data->ptens.pvten_tot;

    double *dvrc_up      = cp->cpcoeffs_pos_dvr[ip_now].dvrc_up;
    double *dvrc_dn      = cp->cpcoeffs_pos_dvr[ip_now].dvrc_dn;
    double *dvrfc_up     = cp->cpcoeffs_pos_dvr[ip_now].dvrfc_up;
    double *dvrfc_dn     = cp->cpcoeffs_pos_dvr[ip_now].dvrfc_dn;

    double *fpc_up      = cp->cpcoeffs_pos_dvr[ip_now].fpc_up;
    double *fpc_dn      = cp->cpcoeffs_pos_dvr[ip_now].fpc_dn;

    int icoef_form_up    = cp->cpcoeffs_pos_dvr[ip_now].icoef_form_up;
    int icoef_orth_up    = cp->cpcoeffs_pos_dvr[ip_now].icoef_orth_up;
    int ifcoef_form_up   = cp->cpcoeffs_pos_dvr[ip_now].ifcoef_form_up;
    int ifcoef_orth_up   = cp->cpcoeffs_pos_dvr[ip_now].ifcoef_orth_up;
    int icoef_form_dn    = cp->cpcoeffs_pos_dvr[ip_now].icoef_form_dn;
    int icoef_orth_dn    = cp->cpcoeffs_pos_dvr[ip_now].icoef_orth_dn;
    int ifcoef_form_dn   = cp->cpcoeffs_pos_dvr[ip_now].ifcoef_form_dn;
    int ifcoef_orth_dn   = cp->cpcoeffs_pos_dvr[ip_now].ifcoef_orth_dn;
    int *ioff_up         = cp->cpcoeffs_info.ioff_upt;
    int *ioff_dn         = cp->cpcoeffs_info.ioff_dnt;

    int nstate_up_proc   = cp->cpcoeffs_info.nstate_up_proc;
    int nstate_dn_proc   = cp->cpcoeffs_info.nstate_dn_proc;

    int *ioff_up_st      = cp->cpcoeffs_info.ioff_up;
    int *ioff_dn_st      = cp->cpcoeffs_info.ioff_dn;
    int icoef_form_tmp;

    int nstate_up        = cp->cpcoeffs_info.nstate_up;
    int nstate_dn        = cp->cpcoeffs_info.nstate_dn;
    int ncoef_up         = cp->cp_comm_state_pkg_dvr_up.nstate_ncoef_proc;
    int ncoef_dn         = cp->cp_comm_state_pkg_dvr_dn.nstate_ncoef_proc;
    int ncoef_up_max     = cp->cp_comm_state_pkg_dvr_up.nstate_ncoef_proc_max;
    int ncoef_dn_max     = cp->cp_comm_state_pkg_dvr_dn.nstate_ncoef_proc_max;
    int ncoef            = cp->cpcoeffs_info.ncoef;

    int cp_lsda          = cp->cpopts.cp_lsda;
    int cg_reset_flag    = cp->cpcoeffs_info.cg_reset_flag;
    int cp_norb          = cp->cpopts.cp_norb;

    int cp_init_min      = cp->cpopts.cp_init_min_opt;
    int iperd            = general_data->cell.iperd;

    int cp_cg_line_min_len = general_data->minopts.cp_cg_line_min_len;
    double *hcre_up, *hcre_dn;
    double *zeta_up,*zeta_dn;

    /* local variables */

    double eenergy,eenergy_temp,dt;

    int iii,i,j,k,icoef,is,icount;
    int ncoef_up_tot,ncoef_dn_tot;
    double gamma_up,gamma_dn;
    double fc_mag_up,fc_mag_dn;
    static double fovlap_up,fovlap_dn;
    static double fovlap_tmp;
    double fovlap_up_old,fovlap_dn_old;

    /*test*/
    int natm_tot  = class->clatoms_info.natm_tot;
    double *class_clatoms_fx   = class->clatoms_pos[ip_now].fx;
    double *class_clatoms_fy   = class->clatoms_pos[ip_now].fy;
    double *class_clatoms_fz   = class->clatoms_pos[ip_now].fz;


/*==========================================================================*/
/* 0) Checks                                                                */

  if(cp_norb>0){
    if((icoef_orth_up+ifcoef_orth_up)!=0){
      printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
      printf("Up CP vectors are in orthonormal form \n");
      printf("on state processor %d in min_CG_cp \n",myid_state);
      printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
      fflush(stdout);
      exit(1);
    }/*endif*/
    if(cp_lsda==1){
     if((icoef_orth_dn+ifcoef_orth_dn)!=0){
       printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
       printf("Dn CP vectors are in orthonormal form \n");
       printf("on state processor %d in min_CG_cp \n",myid_state);
       printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
       fflush(stdout);
       exit(1);
     }/*endif*/
    }/*endif*/
  }/*endif*/

  if(np_states>1){
    if((icoef_form_up+ifcoef_form_up)!=2){
     printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
     printf("Up CP vectors are not in transposed form \n");
     printf("on state processor %d in min_CG_cp \n",myid_state);
     printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
     fflush(stdout);
     exit(1);
    }/*endif*/
    if(cp_lsda==1){
     if((icoef_form_dn+ifcoef_form_dn)!=2){
      printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
      printf("Up CP vectors are not in transposed form \n");
      printf("on state processor %d in min_CG_cp \n",myid_state);
      printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
      fflush(stdout);
      exit(1);
     }/*endif*/
    }/*endif*/
  }/*endif*/

/*==========================================================================*/
/* 0) Useful constants                                                      */

  general_data->timeinfo.int_res_tra    = 0;
  general_data->timeinfo.int_res_ter    = 0;
  dt                                    = general_data->timeinfo.dt;
  general_data->stat_avg.iter_shake     = 0;
  general_data->stat_avg.iter_ratl      = 0;

  ncoef_up_tot = ncoef_up_max*nstate_up;
  ncoef_dn_tot = ncoef_dn_max*nstate_dn;

  zeta_up = (double *) cmalloc(ncoef*sizeof(double))-1;

  if(cp_lsda==1 && nstate_dn != 0){
    zeta_dn = (double *) cmalloc(ncoef*sizeof(double))-1;
  }/*endif*/

  if(cp_init_min == 1){
    hcre_up = (double *) cmalloc(ncoef_up_tot*sizeof(double))-1;
    if(cp_lsda==1 && nstate_dn !=0){
      hcre_dn = (double *) cmalloc(ncoef_dn_tot*sizeof(double))-1;
    }
    dt=dt*20.0;
  }else{
    hcre_up      = cp->cpcoeffs_pos_dvr[ip_now].dvrvc_up;
    hcre_dn      = cp->cpcoeffs_pos_dvr[ip_now].dvrvc_dn;
  }

/*==========================================================================*/
/* 0.1) Zero conjugate gradients                                            */

  if(cg_reset_flag == 1){
    for(i=1;i<=ncoef_up_tot; i++){
      hcre_up[i] = 0.0;
    }
    if( (cp_lsda== 1) && (nstate_dn != 0) ){
      for(i=1;i<=ncoef_dn_tot; i++){
        hcre_dn[i] = 0.0;
      }
    }/* endif */
    gamma_up = 0.0;
    gamma_dn = 0.0;
    fovlap_up = 1.0;
    fovlap_dn = 1.0;
  }/* endif */

/*==========================================================================*/
/* I) Get forces                                                            */

  for(i=1;i<= natm_tot;i++){
    class_clatoms_fx[i]  = 0.0;
    class_clatoms_fy[i]  = 0.0;
    class_clatoms_fz[i]  = 0.0;
  }/*endfor*/

  for(i=1;i<=ncoef_up_tot;i++){
    dvrfc_up[i] = 0.0;
  }/*endfor*/
  if( (cp_lsda== 1) && (nstate_dn != 0) ){
    for(i=1;i<=ncoef_dn_tot; i++){
      dvrfc_dn[i] = 0.0;
    }/*endfor*/
  }/*endif*/
  general_data->stat_avg.cp_ehart = 0.0;
  general_data->stat_avg.cp_exc   = 0.0;
  general_data->stat_avg.cp_muxc  = 0.0;
  general_data->stat_avg.cp_eext  = 0.0;
  general_data->stat_avg.cp_enl   = 0.0;
  general_data->stat_avg.cp_eke   = 0.0;
  general_data->stat_avg.vrecip   = 0.0;

  for(i=1;i<=9;i++){
    ptens_pvten[i]     = 0.0;
    ptens_pvten_tot[i] = 0.0;
  }/*endfor*/

  cp_ks_energy_ctrl(cp,ip_now,&(general_data->ewald),&(class->ewd_scr),
                       &(general_data->cell),
                       &(class->clatoms_info),
                       &(class->clatoms_pos[ip_now]),
                       &(class->atommaps),&(general_data->stat_avg),
                       &(general_data->ptens),
                       &(general_data->simopts),
                       &(class->for_scr));

  eenergy_temp = general_data->stat_avg.cp_ehart
               + general_data->stat_avg.cp_exc
               + general_data->stat_avg.cp_eext
               + general_data->stat_avg.cp_enl
               + general_data->stat_avg.cp_eke;


  if(np_states>1){
    Allreduce(&(eenergy_temp),&(eenergy),1,MPI_DOUBLE,MPI_SUM,0,comm_states);
  }else{
    eenergy = eenergy_temp;
  }

/*==========================================================================*/
/* II) Get auxilary forces using FBR preconditioner                     */

  if (np_states > 1) {
     Barrier(comm_states);
     icoef_form_tmp=1;
     cp_transpose_bck_dvr(dvrfc_up,&(icoef_form_tmp), cp->cpscr.cpscr_wave.cre_up,
                   &(cp->cp_comm_state_pkg_dvr_up),cp->cp_para_fft_pkg3d_sm.nkf1,
                   cp->cp_para_fft_pkg3d_sm.nkf2,cp->cp_para_fft_pkg3d_sm.nkf3);

     if(cp_lsda==1 && nstate_dn !=0){
       Barrier(comm_states);
       icoef_form_tmp=1;
       cp_transpose_bck_dvr(dvrfc_dn,&(icoef_form_tmp),cp->cpscr.cpscr_wave.cre_dn,
                   &(cp->cp_comm_state_pkg_dvr_dn),cp->cp_para_fft_pkg3d_sm.nkf1,
                   cp->cp_para_fft_pkg3d_sm.nkf2,cp->cp_para_fft_pkg3d_sm.nkf3);
     }
  }

  for (i=1;i<=nstate_up_proc;i++){
     get_precon_force_dvr(fpc_up,dvrfc_up,ncoef,&(cp->dvr_matrix),
              &(cp->cpcoeffs_info),&(cp->cpscr.cpscr_wave),ioff_up_st[i],iperd);

  }

  if ((cp_lsda== 1) && (nstate_dn != 0)){
    for (i=1;i<=nstate_dn_proc;i++){
       get_precon_force_dvr(fpc_dn,dvrfc_dn,ncoef,&(cp->dvr_matrix),
            &(cp->cpcoeffs_info),&(cp->cpscr.cpscr_wave),ioff_dn_st[i],iperd);
    }
  }

/*==========================================================================*/
/* III) Calculate the gamma's                                                */

  fovlap_up_old = fovlap_up;
  fovlap_up = 0.0;

  if(np_states>1){
    for(i=1;i<=nstate_up_proc;i++){
      fovlap_up += diag_ovlap_dvr(ncoef,dvrfc_up,fpc_up,ioff_up_st[i]);
    }
    fovlap_tmp = fovlap_up;
    Barrier(comm_states);
    Allreduce(&(fovlap_tmp),&(fovlap_up),1,MPI_DOUBLE,MPI_SUM,0,comm_states);
  }else{
    for(i=1;i<=nstate_up;i++){
      fovlap_up += diag_ovlap_dvr(ncoef,dvrfc_up,fpc_up,ioff_up[i]);
    }
  }

  if(cg_reset_flag != 1) {gamma_up = fovlap_up/fovlap_up_old;}
  //test 
  // printf("cg_reset_flag %d fovlap_up %f fovlap_up_old %f gamm_up %f\n",cg_reset_flag,fovlap_up,fovlap_up_old,gamma_up);

  if( (cp_lsda== 1) && (nstate_dn != 0) ){
    fovlap_dn_old = fovlap_dn;
    fovlap_dn = 0.0;
    if(np_states>1){
      for(i=1;i<=nstate_dn_proc;i++){
        fovlap_dn += diag_ovlap_dvr(ncoef,dvrfc_dn,fpc_dn,ioff_dn_st[i]);
      }
      fovlap_tmp = fovlap_dn;
      Barrier(comm_states);
      Allreduce(&(fovlap_tmp),&(fovlap_dn),1,MPI_DOUBLE,MPI_SUM,0,comm_states);
    }else{
      for(i=1;i<=nstate_up;i++){
        fovlap_dn += diag_ovlap_dvr(ncoef,dvrfc_dn,fpc_dn,ioff_dn_st[i]);
      }
      if(cg_reset_flag != 1) {gamma_dn = fovlap_dn/fovlap_dn_old;}
    }
  }
  //test 
  // printf("cg_reset_flag %d fovlap_dn %f fovlap_dn_old %f gamm_dn %f\n",cg_reset_flag,fovlap_dn,fovlap_dn_old,gamma_dn);


/*==========================================================================*/
/* convert the force into transposed form */

   if (np_states > 1) {
     Barrier(comm_states);
     icoef_form_tmp=0;
     cp_transpose_fwd_dvr(fpc_up,&(icoef_form_tmp),cp->cpscr.cpscr_wave.cre_up,
                  &(cp->cp_comm_state_pkg_dvr_up),cp->cp_para_fft_pkg3d_sm.nkf1,
                  cp->cp_para_fft_pkg3d_sm.nkf2,cp->cp_para_fft_pkg3d_sm.nkf3);

     if( (cp_lsda== 1) && (nstate_dn != 0) ){
       Barrier(comm_states);
       icoef_form_tmp=0;
       cp_transpose_fwd_dvr(fpc_dn,&(icoef_form_tmp),cp->cpscr.cpscr_wave.cre_dn,
                    &(cp->cp_comm_state_pkg_dvr_dn),cp->cp_para_fft_pkg3d_sm.nkf1,
                    cp->cp_para_fft_pkg3d_sm.nkf2,cp->cp_para_fft_pkg3d_sm.nkf3);

     }
   }

/*==========================================================================*/
/* IV) Evolve gradients                                                   */

   for(i=1;i<=ncoef_up*nstate_up; i++){
      hcre_up[i] = fpc_up[i] + gamma_up*hcre_up[i];
      // printf("i %d fpc_up[i] %f gamma_up %f hcre_up[i] %f\n",i,fpc_up[i],gamma_up,hcre_up[i]);
   }/*endfor*/

   if( (cp_lsda== 1) && (nstate_dn != 0) ){
    for(i=1;i<=ncoef_dn*nstate_dn; i++){
      hcre_dn[i] = fpc_dn[i] + gamma_dn*hcre_dn[i];
    }/*endfor*/
   }/* endif */

/*==========================================================================*/
/* V) Calculate the step length                                           */

  if(cp_cg_line_min_len == 0){
   for(i=1;i<=ncoef_up;i++) {
    zeta_up[i] = dt;
   }
    if( (cp_lsda== 1) && (nstate_dn != 0) ){
    for(i=1;i<=ncoef_dn;i++) {
     zeta_dn[i] = dt;
    }
    }
   }else{
     if(myid_state==0){
       printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
       printf("Up CP vectors are in orthonormal form \n");
       printf("on state processor %d in min_CG_cp \n",myid_state);
       printf("@@@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@@\n");
     }
     fflush(stdout);
     exit(1);

    /* line_min_cp_dvr(class,bonded,general_data,cp,zeta_up,
                    zeta_dn,ip_now,eenergy); */
   }/*endif*/

/*==========================================================================*/
/* VI) Evolve positions and coefficients                                   */

   for(is=1;is<=nstate_up;is++) {
     for(i=1;i<=ncoef_up;i++) {
       icoef = i+ioff_up[is];
       dvrc_up[icoef] += zeta_up[i]*hcre_up[icoef];
     }/*endfor*/
   }/*endfor*/

   if( (cp_lsda== 1) && (nstate_dn != 0) ){
     for(is=1;is<=nstate_dn;is++) {
       for(i=1;i<=ncoef_dn;i++) {
         icoef = i+ioff_dn[is];
         dvrc_dn[icoef] += zeta_dn[i]*hcre_dn[icoef];
       }/*endfor*/
     }/*endfor*/
   }/* endif */

/*==========================================================================*/
/* V) Orthogonalize wave functions  */

    orthog_control_cp_dvr(cp,ip_now);

/*==========================================================================*/
/* ii) Free the memory                                                      */

   cfree(&(zeta_up[1]));
   if( (cp_lsda== 1) && (nstate_dn != 0) ){
    cfree(&(zeta_dn[1]));
   }

   if(cp_init_min==1){
     cfree(&(hcre_up[1]));
     if((cp_lsda== 1) && (nstate_dn != 0) ){
       cfree(&(hcre_dn[1]));
     }
   }

/*-------------------------------------------------------------------------*/
    }/*end routine*/
/*==========================================================================*/

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
void get_precon_force_dvr(double *fcre,double *dvrfc,int ncoef,DVR_MATRIX *dvr_matrix,
                      CPCOEFFS_INFO *cpcoeffs_info,CPSCR_WAVE *cpscr_wave,
                      int ioff, int iperd)

/*========================================================================*/
{/*begin routine*/
/*========================================================================*/
/*             Local variable declarations                                */
#include "../typ_defs/typ_mask.h"

  int i,j,k,l,m,n,id,is,ip;
  double sum1,sum2,v_tmp1,v_tmp2;

  int grid_nx  = cpcoeffs_info->grid_nx;
  int grid_ny  = cpcoeffs_info->grid_ny;
  int grid_nz  = cpcoeffs_info->grid_nz;

  double *TRx   = dvr_matrix->TRx;
  double *TRy   = dvr_matrix->TRy;
  double *TRz   = dvr_matrix->TRz;
  double *TIx   = dvr_matrix->TIx;
  double *TIy   = dvr_matrix->TIy;
  double *TIz   = dvr_matrix->TIz;

  double *Tfbr_x = dvr_matrix->Tfbr_x;
  double *Tfbr_y = dvr_matrix->Tfbr_y;
  double *Tfbr_z = dvr_matrix->Tfbr_z;

  double *vr1=  cpscr_wave->zfft;
  double *vr2= &cpscr_wave->zfft[ncoef];
  double *vi1=  cpscr_wave->zfft_tmp;
  double *vi2= &cpscr_wave->zfft_tmp[ncoef];

/*--------------------------------------------------------------------------*/
/* 0) Check on periodicity */

  if(!(iperd==3 || iperd==0)){
    printf("@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@\n");
    printf("  Wrong periodicity in get_precon_force_dvr  \n");
    printf("  iperd = %d\n",iperd);
    printf(" Periodicity should be 0 or 3\n");
    printf("@@@@@@@@@@@@@@@@@@_ERROR_@@@@@@@@@@@@@@@@@@@\n");
    exit(1);
  }

/*----------------------------------------------------------------------------*/
/* I) Transform along the x coordinate (inner most index) */

  id=0;
  for (i=1;i<=grid_nz;i++) {
    for (j=1;j<=grid_ny;j++) {
      ip=(i-1)*grid_nx*grid_ny+(j-1)*grid_nx;
      if(iperd==3){
        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          is=(n-1)*grid_nx;
          sum1=0.0;
          sum2=0.0;
          for (k=1;k<=grid_nx;k++) {
            sum1=sum1+TRx[is+k]*dvrfc[ioff+ip+k];
            sum2=sum2+TIx[is+k]*dvrfc[ioff+ip+k];
          }
          vr1[id]=sum1;
          vi1[id]=-sum2;
        }
      }else if(iperd == 0){
        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          is=(n-1)*grid_nx;
          sum1=0.0;
          for (k=1;k<=grid_nx;k++) {
            sum1=sum1+TRx[is+k]*dvrfc[ioff+ip+k];
          }
          vr1[id]=sum1;
        }
      }
    }
  }

/*----------------------------------------------------------------------------*/
/* II) Transform along the y coordinate */

  id=0;
  for (i=1;i<=grid_nz;i++) {
    for (m=1;m<=grid_ny;m++) {
      is=(m-1)*grid_ny;
      if(iperd==3){

        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          ip=(i-1)*grid_nx*grid_ny+n;

          sum1=0.0;
          sum2=0.0;
          for (j=1;j<=grid_ny;j++) {
            v_tmp1=vr1[ip+(j-1)*grid_nx];
            v_tmp2=vi1[ip+(j-1)*grid_nx];
            sum1=sum1+TRy[is+j]*v_tmp1+TIy[is+j]*v_tmp2;
            sum2=sum2-TIy[is+j]*v_tmp1+TRy[is+j]*v_tmp2;
          }
          vr2[id]=sum1;
          vi2[id]=sum2;
        }
      }else if(iperd==0){
        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          ip=(i-1)*grid_nx*grid_ny+n;

          sum1=0.0;
          for (j=1;j<=grid_ny;j++) {
            sum1=sum1+TRy[is+j]*vr1[ip+(j-1)*grid_nx];
          }
          vr2[id]=sum1;
        }
      }
    }
  }

/*---------------------------------------------------------------------------------*/
/* III) shuffling vector elements so that z becomes the inner most index */

  id=0;
  for (m=1;m<=grid_ny;m++) {
    for (n=1;n<=grid_nx;n++) {
      if(iperd==3){
        for (i=1;i<=grid_nz;i++) {
          id=id+1;
          ip=(i-1)*grid_nx*grid_ny+(m-1)*grid_nx+n;
          vr1[id]=vr2[ip];
          vi1[id]=vi2[ip];
        }
      }else if(iperd==0){
        for (i=1;i<=grid_nz;i++) {
          id=id+1;
          ip=(i-1)*grid_nx*grid_ny+(m-1)*grid_nx+n;
          vr1[id]=vr2[ip];
        }
      }
    }
  }

/*----------------------------------------------------------------------------*/
/* IV) Transform along the z coordinate */

  id=0;
  for (m=1;m<=grid_ny;m++) {
    for (n=1;n<=grid_nx;n++) {
      ip=(m-1)*grid_nx*grid_nz+(n-1)*grid_nz;

      if(iperd==3){
        for (l=1;l<=grid_nz;l++) {
          id=id+1;

          sum1=0.0;
          sum2=0.0;
          is=(l-1)*grid_nz;
          for (i=1;i<=grid_nz;i++) {
            v_tmp1=vr1[ip+i];
            v_tmp2=vi1[ip+i];
            sum1=sum1+TRz[is+i]*v_tmp1+TIz[is+i]*v_tmp2;
            sum2=sum2-TIz[is+i]*v_tmp1+TRz[is+i]*v_tmp2;
          }
          vr2[id]=sum1;
          vi2[id]=sum2;
        }
      }else if(iperd==0){
        for (l=1;l<=grid_nz;l++) {
          id=id+1;

          sum1=0.0;
          is=(l-1)*grid_nz;
          for (i=1;i<=grid_nz;i++) {
            sum1=sum1+TRz[is+i]*vr1[ip+i];
          }
          vr2[id]=sum1;
        }
      }
    }
  }

/*----------------------------------------------------------------------------*/
/* V) multiply inverse of kinetic energy */

  id=0;
  for (m=1;m<=grid_ny;m++) {
    for (n=1;n<=grid_nx;n++) {
      if(iperd==3){
        for (l=1;l<=grid_nz;l++) {
          id=id+1;
          sum1=1.0/(Tfbr_x[n]+Tfbr_y[m]+Tfbr_z[l]);
          vr2[id]=vr2[id]*sum1;
          vi2[id]=vi2[id]*sum1;
        }
      }else if(iperd==0){
        for (l=1;l<=grid_nz;l++) {
          id=id+1;
          sum1=1.0/(Tfbr_x[n]+Tfbr_y[m]+Tfbr_z[l]);
          vr2[id]=vr2[id]*sum1;
        }
      }
    }
  }

/*------------------------------------------------------------------------*/
/* VI) transform along the z coordinate */

  id=0;
  for (m=1;m<=grid_ny;m++) {
    for (n=1;n<=grid_nx;n++) {
      ip=(m-1)*grid_nx*grid_nz+(n-1)*grid_nz;
      if(iperd==3){
        for (i=1;i<=grid_nz;i++) {
          id=id+1;
          sum1=0.0;
          sum2=0.0;
          for (l=1;l<=grid_nz;l++) {
            v_tmp1=vr2[ip+l];
            v_tmp2=vi2[ip+l];
            is=(l-1)*grid_nz+i;
            sum1=sum1+TRz[is]*v_tmp1-TIz[is]*v_tmp2;
            sum2=sum2+TIz[is]*v_tmp1+TRz[is]*v_tmp2;
          }
          vr1[id]=sum1;
          vi1[id]=sum2;
        }
      }else if(iperd==0){
        for (i=1;i<=grid_nz;i++) {
          id=id+1;
          sum1=0.0;
          for (l=1;l<=grid_nz;l++) {
            is=(l-1)*grid_nz+i;
            sum1=sum1+TRz[is]*vr2[ip+l];
          }
          vr1[id]=sum1;
        }
      }
    }
  }

/*---------------------------------------------------------------------*/
/* VII) shuffling back to the original order */

  id=0;
  for (i=1;i<=grid_nz;i++) {
    for (m=1;m<=grid_ny;m++) {
      if(iperd==3){
        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          ip=(m-1)*grid_nz*grid_nx+(n-1)*grid_nz+i;
          vr2[id]=vr1[ip];
          vi2[id]=vi1[ip];
        }
      }else if(iperd==0){
        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          ip=(m-1)*grid_nz*grid_nx+(n-1)*grid_nz+i;
          vr2[id]=vr1[ip];
        }
      }
    }
  }

/*------------------------------------------------------------------*/
/* VIII) transform along the y coordinate */

  id=0;
  for (i=1;i<=grid_nz;i++) {
    for (j=1;j<=grid_ny;j++) {
      if(iperd==3){
        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          sum1=0.0;
          sum2=0.0;
          for (m=1;m<=grid_ny;m++) {
            ip=(i-1)*grid_nx*grid_ny+(m-1)*grid_nx+n;
            v_tmp1=vr2[ip];
            v_tmp2=vi2[ip];
            is=(m-1)*grid_ny+j;
            sum1=sum1+TRy[is]*v_tmp1-TIy[is]*v_tmp2;
            sum2=sum2+TIy[is]*v_tmp1+TRy[is]*v_tmp2;
          }
          vr1[id]=sum1;
          vi1[id]=sum2;
        }
      }else if(iperd==0){
        for (n=1;n<=grid_nx;n++) {
          id=id+1;
          sum1=0.0;
          for (m=1;m<=grid_ny;m++) {
            ip=(i-1)*grid_nx*grid_ny+(m-1)*grid_nx+n;
            is=(m-1)*grid_ny+j;
            sum1=sum1+TRy[is]*vr2[ip];
          }
          vr1[id]=sum1;
        }
      }
    }
  }

/*------------------------------------------------------------------*/
/* IX) transform along the x coordinate */

  id=0;
  for (i=1;i<=grid_nz;i++) {
    for (j=1;j<=grid_ny;j++) {
      ip=(i-1)*grid_nx*grid_ny+(j-1)*grid_nx;
      if(iperd==3){
        for (k=1;k<=grid_nx;k++) {
          id=id+1;
          sum1=0.0;
          for (n=1;n<=grid_nx;n++) {
            is=(n-1)*grid_nx+k;
            sum1=sum1+TRx[is]*vr1[ip+n]-TIx[is]*vi1[ip+n];
          }
          fcre[ioff+id]=sum1;
        }
      }else if(iperd==0){
        for (k=1;k<=grid_nx;k++) {
          id=id+1;
          sum1=0.0;
          for (n=1;n<=grid_nx;n++) {
            is=(n-1)*grid_nx+k;
            sum1=sum1+TRx[is]*vr1[ip+n];
          }
          fcre[ioff+id]=sum1;
        }
      }
    }
  }

/*-------------------------------------------------------------------------*/
}/*end routine*/
/*==========================================================================*/

double diag_ovlap_dvr(int ncomp,double *v1_re,double *v2_re,int ioff)
/*========================================================================*/
{/*begin routine */
/*========================================================================*/

 int ig,i;
 double ovlap;

 ovlap = 0.0;
 for(ig=1;ig<=ncomp;ig++){
  i = ig+ioff;
  ovlap += v1_re[i]*v2_re[i];
 }

  return ovlap;

/*========================================================================*/
}/* end routine */
/*========================================================================*/

