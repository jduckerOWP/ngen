#include "Formulation.hpp"
#include <JSONProperty.hpp>
#include <exception>

#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>

// Formulations
#include "Formulation_Constructors.hpp"
#include "Bmi_Cpp_Formulation.hpp"
#include "Bmi_C_Formulation.hpp"
#include "Bmi_Fortran_Formulation.hpp"
#include "Bmi_Multi_Formulation.hpp"
#include "Bmi_Py_Formulation.hpp"
#include <GenericDataProvider.hpp>
#include "CsvPerFeatureForcingProvider.hpp"
#include "NullForcingProvider.hpp"
#ifdef NETCDF_ACTIVE
    #include "NetCDFPerFeatureDataProvider.hpp"
#endif

namespace realization {
    template<class T>
    static constructor create_formulation_constructor() {
        return [](std::string id, std::shared_ptr<data_access::GenericDataProvider> forcing_provider, utils::StreamHandler output_stream) -> std::shared_ptr<Catchment_Formulation>{
            return std::make_shared<T>(id, forcing_provider, output_stream);
        };
    }

    std::map<std::string, constructor> formulations = {
        {"bmi_c++", create_formulation_constructor<Bmi_Cpp_Formulation>()},
#ifdef NGEN_BMI_C_LIB_ACTIVE
        {"bmi_c", create_formulation_constructor<Bmi_C_Formulation>()},
#endif // NGEN_BMI_C_LIB_ACTIVE
#ifdef NGEN_BMI_FORTRAN_ACTIVE
        {"bmi_fortran", create_formulation_constructor<Bmi_Fortran_Formulation>()},
#endif // NGEN_BMI_FORTRAN_ACTIVE
        {"bmi_multi", create_formulation_constructor<Bmi_Multi_Formulation>()},
#ifdef ACTIVATE_PYTHON
        {"bmi_python", create_formulation_constructor<Bmi_Py_Formulation>()},
#endif // ACTIVATE_PYTHON
    };
}
