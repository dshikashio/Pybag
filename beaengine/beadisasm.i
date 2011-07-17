%module beadisasm
%{
#define SWIG_FILE_WITH_INIT
#include "BeaEngine.h"
%}
%include <windows.i>
%include "BeaEngine.h"