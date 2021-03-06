#if defined(FORTRANUNDERSCORE)
#define ZFFT1DI zfft1di_
#define ZFFT1D zfft1d_
#define RS rs_
#define RSG rsg_
#define DURAND durand_
#define DGEFA dgefa_
#define DGESL dgesl_
#define DGEMM dgemm_
#define DFFTS dffts_
#define ZFFTS zffts_
#define DCFT dcft_
#define CFTFAX cftfax_
#define DCFFTI_GENERIC dcffti_generic_
#define DCFFTF_GENERIC dcfftf_generic_
#define DCFFTB_GENERIC dcfftb_generic_
#define GEN_MATMUL gen_matmul_
#define DDOT ddot_
#define DAXPY daxpy_
#define DSCAL dscal_
#define GAUSSQ gaussq_
#define LBFGS lbfgs_
#define ZHETRD zhetrd_
#define ZUNGTR zungtr_
#define ZSTEQR zsteqr_
#define ZHEEVR zheevr_
#define ZHEEVD zheevd_
#define ZSIFA zsifa_
#define ZSIDI zsidi_
#elif defined(_CRAY)
#define RS RS
#define RSG RSG
#define DURAND DURAND
#define DGEFA SGEFA
#define DGESL SGESL
#define DGEMM dgemm
#define DFFTS DFFTS
#define ZFFTS ZFFTS
#define DCFT DCFT
#define CFTFAX CFTFAX
#define ZFFT1DI ZFFT1DI
#define ZFFT1D ZFFT1D
#define DCFFTI_GENERIC DCFFTI_GENERIC
#define DCFFTF_GENERIC DCFFTF_GENERIC
#define DCFFTB_GENERIC DCFFTB_GENERIC
#define CFFTI_GENERIC CFFTI_GENERIC
#define CFFTF_GENERIC CFFTF_GENERIC
#define CFFTB_GENERIC CFFTB_GENERIC
#define GEN_MATMUL GEN_MATMUL
#define DDOT DDOT
#define DAXPY DAXPY
#define DSCAL DSCAL
#define GAUSSQ GAUSSQ
#define LBFGS LBFGS
#define ZHETRD ZHETRD 
#define ZUNGTR ZUNGTR
#define ZSTEQR ZSTEQR
#define ZHEEVR ZHEEVR
#define ZHEEVD ZHEEVD
#define ZSIFA ZSIFA
#define ZSIDI ZSIDI
#else
#define RS rs
#define RSG rsg
#define DURAND durand
#define DGEFA dgefa
#define DGESL dgesl
#define DGEMM dgemm
#define DFFTS dffts
#define ZFFTS zffts
#define DCFT dcft
#define CFTFAX cftfax
#define ZFFT1DI zfft1di
#define ZFFT1D zfft1d
#define DCFFTI_GENERIC dcffti_generic
#define DCFFTF_GENERIC dcfftf_generic
#define DCFFTB_GENERIC dcfftb_generic
#define GEN_MATMUL gen_matmul
#define DDOT ddot
#define DAXPY daxpy
#define DSCAL dscal
#define GAUSSQ gaussq
#define LBFGS lbfgs
#define ZHETRD zhetrd
#define ZUNGTR zungtr
#define ZSTEQR zsteqr
#define ZHEEVR zheevr
#define ZHEEVD zheevd
#define ZSIFA zsifa
#define ZSIDI zsidi
#endif

/*----------------------------------------------------------------------*/

void DGEFA(double *, int *, int *, int *, int * );

void DGESL(double *, int *, int *, int *, double *, int * );

void DGEMM (char *, char *, int *, int *, int *, double *, double *, 
                  int *, double *, int *, double *, double *, int * );

void CFTFAX(int *,int *,double *);

void CFFT99(double *,double *,double *,
           int *,int *,int *,int *,int *,int *);

void DCFT(int *,double *,int *,int *,
          double *, int *,int *,int *,int *,int *,double *,
          double *,int *,double *,int *);

void ZFFT1DI(int *,double *);

void ZFFT1D(int *,int *,double *,int *,double *);

void DFFTS(double *, double *, int *, int *, int *, int *, int *, int * );

void ZFFTS(double *, int *, int *, int *, int *, int *, int * );

void DCFFTI_GENERIC(int *,double *);

void DCFFTF_GENERIC(int *,double *,double *);

void DCFFTB_GENERIC(int *,double *,double *);

void RS(int *,int *,double [],double [],int *,double [],
        double [],double [],int *);

void RSG(int *,int *,double [],double [],double [],int *,double [],
         double [],double [],int *);

double DDOT(int *,double *,int *,double *,int *);

void DAXPY(int *,double *,double *,int *,double *,int *);

void DSCAL(int *,double *,double *,int *);

void GAUSSQ(int *, int *, double *, double *, int *, double *, double *,
            double *, double *);

/*----------------------------------------------------------------------*/

void gethinv(double *, double *, double *, int );

double getdeth(double *);

double ddot1(int ,double *,int,double *,int);

double dsum1(int ,double *,int);

double gerf(double);

double gerfc(double);

double surf_corr(double);

double dsurf_corr(double);

double d2surf_corr(double);

void matmul_2(double *, double *, double *, int );

void matmul_2s(double *, double *, int );

void matmul_3(double *, double *);

void matmul_tt(double *, double *, double *, int );

void matmul_t(double *, double *, double *, int );

void matmul_t2(double *, double *, double *, int );

void diag33(double *, double *, double *, double *, double *);

void cputime(double *time);

void  par_cpu_vomit(double ,MPI_Comm ,int ,int ,char []);
 
/*----------------------------------------------------------------------*/

void gaussran(int, int *, int *, double *, double *);

double ran_essl(double *);

void DURAND(double *,int *, double *,int *);

/*----------------------------------------------------------------------*/

void  GEN_MATMUL(double *,int *,int *,double *,int *,int *,
                 double *,int *,int *,int *,int *,
                 double *,double *);

/*----------------------------------------------------------------------*/

void setfft_indx(int ,int ,int ,int ,int *,int *,int *,int *,int *);

void sort_commence(int , int [],int []);

void create_para_fft_pkg3d(PARA_FFT_PKG3D *,int *,int *,int *,int );

void destroy_para_fft_pkg3d(PARA_FFT_PKG3D *);

void para_fft_gen3d_fwd_to_r(double *,double *,PARA_FFT_PKG3D *);

void para_fft_gen3d_bck_to_g(double *,double *,PARA_FFT_PKG3D *);

void sngl_pack_coef(double *,double *,double *,PARA_FFT_PKG3D *);

void pme_sngl_pack_coef(double *,double *,double *,PARA_FFT_PKG3D *);

void dble_pack_coef(double *, double *,double *, double *,double *,
                    PARA_FFT_PKG3D *);

void para_fft_gen3d_bck_to_g(double *, double *,PARA_FFT_PKG3D *);

void sngl_upack_coef(double *,double *,double *,PARA_FFT_PKG3D *);

void sngl_upack_coef_sum(double *,double *,double *,PARA_FFT_PKG3D *);

void sngl_upack_rho(double *,double *,PARA_FFT_PKG3D *);

void sngl_upack_rho_sum(double *,double *,PARA_FFT_PKG3D *);

void sngl_upack_rho_scal(double *,double *,PARA_FFT_PKG3D *,double);

void sngl_pack_rho(double *,double *,PARA_FFT_PKG3D *);

void sngl_pack_rho_dual_ser(double *,double *,CELL *,double  ,
                           PARA_FFT_PKG3D *,PARA_FFT_PKG3D *);



void sngl_pack_rho_dual_par(double *,double *,double *,int *,int ,CELL *,double ,
                           PARA_FFT_PKG3D *,PARA_FFT_PKG3D *);

void sngl_upack_rho_dual_ser(double *,double *,CELL *,
                            PARA_FFT_PKG3D *,PARA_FFT_PKG3D *);

void sngl_upack_rho_dual_par(double *,double *,double *,int *,int ,CELL *,
                            PARA_FFT_PKG3D *,PARA_FFT_PKG3D *);


void dble_upack_rho(double *,double *,double *,PARA_FFT_PKG3D *);

void dble_upack_rho_scal(double *,double *,double *,PARA_FFT_PKG3D *,double);

void dble_pack_rho(double *,double *,double *,PARA_FFT_PKG3D *);

void dble_upack_coef(double *,double *,double *,double *,double *,
                     PARA_FFT_PKG3D *);

void dble_upack_coef_sum(double *,double *,double *,double *,double *,
                         PARA_FFT_PKG3D *);

void sum_rho(double *,double *,PARA_FFT_PKG3D *);

void para_fft_gen3d_init(PARA_FFT_PKG3D *);
/*==============MA=================*/
void para_fft_gen3d_init_dvr(PARA_FFT_PKG3D *);

void fft_gen1d(double *,int ,int ,int ,int ,int ,int *,double *,int ,
               double *,int ,int *,int );

void fft_gen1d_init(int ,int ,int, int ,int ,int *,double *,int ,
                    double *,int ,int *,int *, int );

void create_pme_comm_full_g(int ,PARA_FFT_PKG3D *);
void create_pme_comm_hybr(int ,int , MPI_Comm, PARA_FFT_PKG3D *);

/*----------------------------------------------------------------------*/
/* DVR related routines */

void para_fft_gen3d_dvr(double *, double *,PARA_FFT_PKG3D *,int );
void para_fft_gen3d_dvr_bck(double *, double *,PARA_FFT_PKG3D *,int );

/*----------------------------------------------------------------------*/
/* LAPACK routines */

void LBFGS(int *,int *,double *,double *,double *,int *,double *,int *,
            double *,double *,double *,int *, int *);

void ZHETRD(char *,int *,zomplex *,int *,double *,double *,zomplex *,zomplex *,
             int *,int *);
void ZUNGTR(char *,int *,zomplex *,int *,zomplex *,zomplex *,int *,int *);
void ZSTEQR(char *,int *,double *,double *,zomplex *,int *,double *,int *);
void ZHEEVR(char *,char *,char *,int *,zomplex *,int *,double *,double *,int *,
             int *,double *,int *,double *,zomplex *,int *,int *,zomplex *,int *,
             double *,int *,int *,int *,int *);  /* used in Lapack 3.0 */
void ZHEEVD(char *,char *,int *,zomplex *,int *,double *,zomplex *,int *,double *,
             int *,int *,int *,int *);

void ZSIFA(zomplex *,int *,int *,int *,int *);
void ZSIDI(zomplex *,int *,int *,int *,zomplex *,zomplex *,int *);
/*----------------------------------------------------------------------*/
/* free energy related  */
int get_ind(int*,int*,int);
void get_ind_rev(int*,int*,int,int);


