/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <symbolic/casadi.hpp>
#include <integration/rk_integrator.hpp>
#include <integration/collocation_integrator.hpp>
#include <interfaces/sundials/cvodes_integrator.hpp>
#include <interfaces/sundials/idas_integrator.hpp>
#include <interfaces/sundials/kinsol_solver.hpp>
#include <interfaces/csparse/csparse.hpp>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace CasADi;

/** \brief Generate a simple ODE */
void simpleODE(FX& ffcn, double& tf, vector<double>& x0, double& u0){
  // Time 
  SXMatrix t = ssym("t");
  
  // Parameter
  SXMatrix u = ssym("u");
  
  // Differential states
  SXMatrix s = ssym("s"), v = ssym("v"), m = ssym("m");
  SXMatrix x;
  x.append(s);
  x.append(v);
  x.append(m); 
  
  // Constants
  double alpha = 0.05; // friction
  double beta = 0.1;   // fuel consumption rate
  
  // Differential equation
  SXMatrix ode;
  ode.append(v);
  ode.append((u-alpha*v*v)/m);
  ode.append(-beta*u*u); 
      
  // Quadrature
  SXMatrix quad = pow(v,3) + pow((3-sin(t))-u,2);

  // Callback function
  ffcn = SXFunction(daeIn("t",t,"x",x,"p",u),daeOut("ode",ode,"quad",quad));

  // End time
  tf = 0.5;

  // Initial value
  x0.resize(3);
  x0[0] = 0;
  x0[1] = 0;
  x0[2] = 1;
  
  // Parameter
  u0 = 0.4;
}

/** \brief Generate a simple DAE */
void simpleDAE(FX& ffcn, double& tf, vector<double>& x0, double& u0){
  // Parameter
  SXMatrix u = ssym("u");
  
  // Differential state
  SXMatrix x = ssym("x");

  // Algebraic variable
  SXMatrix z = ssym("z");

  // Differential equation
  SXMatrix ode = -x + 0.5*x*x + u + 0.5*z;

  // Algebraic constraint
  SXMatrix alg = z + exp(z) - 1. + x;

  // Quadrature
  SXMatrix quad = x*x + 3.0*u*u;
  
  // Callback function
  ffcn = SXFunction(daeIn("x",x,"z",z,"p",u),daeOut("ode",ode,"alg",alg,"quad",quad));
  
  // End time
  tf = 5;

  // Initial value
  x0.resize(1);
  x0[0] = 1;
  
  // Parameter
  u0 = 0.4;
}


int main(){
  // For all problems
  enum Problems{ODE,DAE,NUM_PROBLEMS};
  for(int problem=0; problem<NUM_PROBLEMS; ++problem){
    
    // Get problem
    FX ffcn;              // Callback function
    vector<double> x0;    // Initial value
    double u0;            // Parameter value
    double tf;            // End time
    switch(problem){
      case ODE:
        cout << endl << "** Testing ODE example **" << endl;
        simpleODE(ffcn,tf,x0,u0);
        break;
      case DAE:
        cout << endl << "** Testing DAE example **" << endl;
        simpleDAE(ffcn,tf,x0,u0);
        break;
    }
    
    // For all integrators
    enum Integrators{CVODES,IDAS,COLLOCATION,RUNGEKUTTA,NUM_INTEGRATORS};
    for(int integrator=0; integrator<NUM_INTEGRATORS; ++integrator){

      // Get integrator
      Integrator I;
      switch(integrator){
      case CVODES:
        if(problem==DAE) continue; // Skip if DAE
        cout << endl << "== CVodesIntegrator == " << endl;
        I = CVodesIntegrator(ffcn);
        break;
      case IDAS:
        cout << endl << "== IdasIntegrator == " << endl;
        I = IdasIntegrator(ffcn);
        break;
      case RUNGEKUTTA:
        if(problem==DAE) continue; // Skip if DAE
        cout << endl << "== RKIntegrator == " << endl;
        I = RKIntegrator(ffcn);
        break;
      case COLLOCATION:
        cout << endl << "== CollocationIntegrator == " << endl;
        I = CollocationIntegrator(ffcn);
          
        // Set collocation integrator specific options
        I.setOption("expand_f",true);
        I.setOption("implicit_solver",KinsolSolver::creator);
        Dictionary kinsol_options;
        kinsol_options["linear_solver"] = CSparse::creator;
        I.setOption("implicit_solver_options",kinsol_options);
        break;
      }
      
      // Set common options
      I.setOption("tf",tf);
      
      // Initialize the integrator
      I.init();
      
      // Integrate to get results
      I.setInput(x0,"x0");
      I.setInput(u0,"p");
      I.evaluate();
      DMatrix xf = I.output("xf");
      DMatrix qf = I.output("qf");
      cout << setw(50) << "Unperturbed solution: " << "xf  = " << xf <<  ", qf  = " << qf << endl;

      // Perturb solution to get a finite difference approximation
      double h = 0.001;
      I.setInput(u0+h,"p");
      I.evaluate();
      DMatrix xf_pert = I.output("xf");
      DMatrix qf_pert = I.output("qf");
      cout << setw(50) << "Finite difference approximation: " << "d(xf)/d(p) = " << (xf_pert-xf)/h << ", d(qf)/d(p) = " << (qf_pert-qf)/h << endl;

      // Calculate once, forward
      FX I_fwd = I.derivative(1,0);
      I_fwd.setInput(x0,"der_x0");
      I_fwd.setInput(u0,"der_p");
      I_fwd.setInput(0.0,"fwd0_x0");
      I_fwd.setInput(1.0,"fwd0_p");
      I_fwd.evaluate();
      DMatrix fwd_xf = I_fwd.output("fwd0_xf");
      DMatrix fwd_qf = I_fwd.output("fwd0_qf");
      cout << setw(50) << "Forward sensitivities: " << "d(xf)/d(p) = " << fwd_xf << ", d(qf)/d(p) = " << fwd_qf << endl;

      // Calculate once, adjoint
      FX I_adj = I.derivative(0,1);
      I_adj.setInput(x0,"der_x0");
      I_adj.setInput(u0,"der_p");
      I_adj.setInput(0.0,"adj0_xf");
      I_adj.setInput(1.0,"adj0_qf");
      I_adj.evaluate();
      DMatrix adj_x0 = I_adj.output("adj0_x0");
      DMatrix adj_p = I_adj.output("adj0_p");
      cout << setw(50) << "Adjoint sensitivities: " << "d(qf)/d(x0) = " << adj_x0 << ", d(qf)/d(p) = " << adj_p << endl;

      // Perturb adjoint solution to get a finite difference approximation of the second order sensitivities
      I_adj.setInput(x0,"der_x0");
      I_adj.setInput(u0+h,"der_p");
      I_adj.setInput(0.0,"adj0_xf");
      I_adj.setInput(1.0,"adj0_qf");
      I_adj.evaluate();
      DMatrix adj_x0_pert = I_adj.output("adj0_x0");
      DMatrix adj_p_pert = I_adj.output("adj0_p");
      cout << setw(50) << "FD of adjoint sensitivities: " << "d2(qf)/d(x0)d(p) = " << (adj_x0_pert-adj_x0)/h << ", d2(qf)/d(p)d(p) = " << (adj_p_pert-adj_p)/h << endl;
      
      // Forward over adjoint to get the second order sensitivities
      FX I_foa = I_adj.derivative(1,0);
      I_foa.setInput(x0,"der_der_x0");
      I_foa.setInput(u0,"der_der_p");
      I_foa.setInput(1.0,"fwd0_der_p");
      I_foa.setInput(0.0,"der_adj0_xf");
      I_foa.setInput(1.0,"der_adj0_qf");
      I_foa.evaluate();
      DMatrix fwd_adj_x0 = I_foa.output("fwd0_adj0_x0");
      DMatrix fwd_adj_p = I_foa.output("fwd0_adj0_p");
      cout << setw(50) << "Forward over adjoint sensitivities: " << "d2(qf)/d(x0)d(p) = " << fwd_adj_x0 << ", d2(qf)/d(p)d(p) = " << fwd_adj_p << endl;

      // Adjoint over adjoint to get the second order sensitivities
      FX I_aoa = I_adj.derivative(0,1);
      I_aoa.setInput(x0,"der_der_x0");
      I_aoa.setInput(u0,"der_der_p");
      I_aoa.setInput(0.0,"der_adj0_xf");
      I_aoa.setInput(1.0,"der_adj0_qf");
      I_aoa.setInput(1.0,"adj0_adj0_p");
      I_aoa.evaluate();
      DMatrix adj_adj_x0 = I_aoa.output("adj0_der_x0");
      DMatrix adj_adj_p = I_aoa.output("adj0_der_p");
      cout << setw(50) << "Adjoint over adjoint sensitivities: " << "d2(qf)/d(x0)d(p) = " << adj_adj_x0 << ", d2(qf)/d(p)d(p) = " << adj_adj_p << endl;
    }
  }
  return 0;
}
