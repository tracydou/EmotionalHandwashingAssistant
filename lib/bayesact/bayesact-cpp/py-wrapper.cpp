#include <Python.h>
#include <numpy/arrayobject.h>
#include <boost/python.hpp>

#include <exception>

#include <string>
#include <fstream>

#define _PYTHON // Flag for headers to impliment python functions
                //   Note it is defined above the headers intentionally.

#include "agent.h"
#include "transientdynamics.h"
#include "taumappings.h"

#include "values.h"


using namespace bayesact;

template <class X_t>
Agent<X_t>::Agent(std::string tdynFilename, PyObject* settings_p)
{
  Settings* settings = NULL;

  if (settings_p != NULL) {
    // TODO Deal with settings
  }

  std::fstream f(tdynFilename.c_str(), std::fstream::in);
  if (!f.good()) {
    f.close();
    throw new std::exception();
  }

  TransientDynamics* tdyn = TransientDynamics::parseStream(f);
  f.close();

  TauMappings* tauMappings = new TauMappings(tdyn);

  this->init_(tauMappings, settings);
}

template <class X_t>
void
Agent<X_t>::propagateForward_python (PyObject* aab_p, PyObject* observ_p, 
                             PyObject* xobserv_p, PyObject* paab_p,
                             bool verb)
{
  EpaVector* aab;
  EpaVector* observ;
  XVar* xobserv;
  EpaVector* paab;
  aab =     aab_p != NULL ?
            new EpaVector((double *) PyArray_DATA(aab_p)) :
            NULL;
  observ =  observ_p != NULL ? 
            new EpaVector((double *) PyArray_DATA(observ_p)) :
            NULL;
  xobserv = xobserv_p != NULL ?
            NULL : // TODO Impliment XVar flow
            NULL;
  paab =    paab_p != NULL ?
            new EpaVector((double *) PyArray_DATA(paab_p)) :
            NULL;

  this->propagateForward(aab, observ, xobserv, paab, verb);
}

boost::python::tuple 
TauMappings::computeHC_python(PyObject* tau_p)
{
  TauMappings::H* h;
  TauMappings::C* c;

  YVector tau((double *) PyArray_DATA(tau_p));

  this->computeHC(tau, h, c);

  return boost::python::make_tuple<TauMappings::H, TauMappings::C>(*h, *c);

  //TODO Release memory
}

using namespace boost::python;

BOOST_PYTHON_MODULE(bayesactagent)
{
    class_< Agent<XVar> > ("AgentCpp", init<std::string, PyObject*>())
        .def("propagateForward", &Agent<XVar>::propagateForward_python)
    ;
    class_< TransientDynamics > ("TransientDynamicsCpp", init<std::string>() )
    ;
    class_< TauMappings > ("TauMappingsCpp", init<TransientDynamics*>() )
        .def("computeHC", &TauMappings::computeHC_python)
    ;
}
