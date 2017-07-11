
/*******************************************************/
/* CUDF solver: scoeff_solver.h                        */
/* Templates to handle constraint coefficients         */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// Sparse coefficient handling for solvers

#ifndef _SCOEFF_SOLVER_H
#define _SCOEFF_SOLVER_H

#include <cudf.h>

// Template to allow coefficient saving
// mainly used to save objective coefficients
template <typename coeffT, int first_coeff_index, int first_tab_index> class saved_coefficients {
 public:
  int nb_coeffs;         // number of saved coefficients
  int *sindex;           // column number (within the solver) of each coefficient
  coeffT *coefficients;  // saved coefficients

  // return k-th coefficient
  inline coeffT get_coeff(int k) { return coefficients[k]; };
  // return k-th index
  inline int get_index(int k) { return sindex[k]; };

  // do save the coefficients
  saved_coefficients(int nb_coeffs, int *sindex, coeffT *coefficients) {
    int n = nb_coeffs + first_tab_index;

    this->nb_coeffs = nb_coeffs;

    if ((this->sindex = (int *)malloc(n*sizeof(int))) == 0) {
      fprintf(stderr, "saved_coefficients: new: not enough memory to create rindex.\n");
      exit(-1);
    }

    if ((this->coefficients = (coeffT *)malloc(n*sizeof(coeffT))) == 0) {
      fprintf(stderr, "saved_coefficients: new: not enough memory to create coefficients.\n");
      exit(-1);
    }

    for (int k = 0; k < n; k++) {
      this->sindex[k] = sindex[k];
      this->coefficients[k] = coefficients[k];
    }
  };

  ~saved_coefficients() {
    free(sindex);
    free(coefficients);
  };
};

// Template used to manage the constraints and objective coefficient for a give solver
// using sparse matrix
// coeffT gives the type of coefficients
// first_coeff_index gives the first index value used by the solver to store coefficients
// first_tab_index gives the first index value used to store coefficients in the solver (some solvers do not use the 0)
template <typename coeffT, int first_coeff_index, int first_tab_index> class scoeff_solver {
 public:
  int nb_vars;            // total amount of variables (columns)
  int nb_coeffs;          // current number of stored coefficients
  int *tindex;            // allow to get a coefficient from its index within the problem (=-1 for unstored coefficients) 
  int *sindex;            // column number (within the solver) of each coefficient
  coeffT *coefficients;   // stored coefficients

  // Reset coefficients (required before handling another set of coefficients)
  void reset_coeffs(void) {
    for (int k = first_tab_index; k < nb_coeffs + first_tab_index; k++) tindex[sindex[k] - first_coeff_index] = -1;
    nb_coeffs = 0;
  };

  // Return the coefficient of a given package (-1 if not stored)
  coeffT get_coeff(CUDFVersionedPackage *package) { 
    return get_coeff(package->rank); 
  };

  // Return the coefficient of a given rank (column) (-1 if not stored)
  coeffT get_coeff(int rank) {
    if (tindex[rank] == -1) 
      return 0;
    else
      return coefficients[tindex[rank]]; 
  };

  // Set the coefficient of a given package to value
  void set_coeff(CUDFVersionedPackage *package, coeffT value) {
    set_coeff(package->rank, value);
  };

  // Set the coefficient of a given rank (column) to value
  void set_coeff(int rank, coeffT value) {
    if (tindex[rank] == -1) {
      int k = nb_coeffs + first_tab_index;
      tindex[rank] = k ;
      sindex[k] = rank + first_coeff_index;
      coefficients[k] = value;
      nb_coeffs++;
    } else {
      coefficients[tindex[rank]] = value;
    }
  };

  // Initialize the structures used to store the coefficient
  void initialize_coeffs(int nb_vars) {
    int n = nb_vars + first_tab_index;

    this->nb_vars = nb_vars;
    nb_coeffs = 0;
   
    if ((tindex = (int *)malloc(n*sizeof(int))) == 0) {
      fprintf(stderr, "scoeff_solvers: new: not enough memory to create tindex.\n");
      exit(-1);
    }

    for (int k = 0; k < n; k++) tindex[k] = -1;
    
    if ((sindex = (int *)malloc(n*sizeof(int))) == 0) {
      fprintf(stderr, "scoeff_solvers: new: not enough memory to create rindex.\n");
      exit(-1);
    }
    
    if ((coefficients = (coeffT *)malloc(n*sizeof(coeffT))) == 0) {
      fprintf(stderr, "scoeff_solvers: new: not enough memory to create coefficients.\n");
      exit(-1);
    }
  };

  // Store the multiple objective functions
  vector<saved_coefficients<coeffT, first_tab_index, first_coeff_index> *> objectives;

  // Save a new objective function
  void push_obj() { objectives.push_back(new saved_coefficients<coeffT, first_tab_index, first_coeff_index>(nb_coeffs, sindex, coefficients)); }

  // new
  scoeff_solver() { };

  // destructor
  ~scoeff_solver() {
    free(tindex);
    free(sindex);
    free(coefficients);
  };
  
};


#endif
