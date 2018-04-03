#include "constants.h"

const double Constants::pi      = 3.14159265359;

//Fundamental physical constants from NIST database
// Committee on Data for Science and Technology
// https://physics.nist.gov/cuu/Constants/international.html
// "2014 CODATA recommended values"

// Universal constants
const double Constants::c_0     = 2.99792458E8;           // [m/s] - speed of light in vacuum
const double Constants::h       = 6.62607004E-34;         // [Js] - Planck constant


// Physico-chemical constants
const double Constants::k_B     = 1.38064852E-23;         // [J/K] - Boltzmann constant
const double Constants::N_A     = 6.022140857E23;         // [1/mol] - Avogadro constant
const double Constants::R       = 8.3144598;              // [J/mol/K)] - molar gas constant
const double Constants::sigma   = 5.670367E-8;            // [W/m^2/K^4] - Stefan-Boltzmann constant


const double Constants::c_1     = 2.0 * h * c_0 * c_0;    // [Wm^2] - first radiation constant for spectral radiance
const double Constants::c_2     = h * c_0 / k_B;          // [Km] - second radiation constant
