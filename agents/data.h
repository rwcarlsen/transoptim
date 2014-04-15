
#include <map>
#include <string>

#include "cyclus.h"

namespace pyne {

typedef struct simple_xs_struct {
  int nuc;
  double sigma_t;
  double sigma_s;
  double sigma_e;
  double sigma_i;
  double sigma_a;
  double sigma_gamma;
  double sigma_f;
  double sigma_alpha;
  double sigma_proton;
  double sigma_deut;
  double sigma_trit;
  double sigma_2n;
  double sigma_3n;
  double sigma_4n;
} simple_xs_struct;

/// Custom exception for declaring a simple_xs request invalid
class InvalidSimpleXS : public std::exception {
 public:
  InvalidSimpleXS () {};
  ~InvalidSimpleXS () throw () {};

  InvalidSimpleXS(std::string msg) : msg_(msg) {};

  virtual const char* what() const throw() {
    return msg_.c_str();
  };

 private:
  std::string msg_;
};

std::map<std::string, std::map<int, std::map<int, double> > > simple_xs_map;

// loads the simple cross section data for the specified energy band from
// the nuc_data.h5 file into memory.
static void _load_simple_xs_map(std::string energy) {
  //Check to see if the file is in HDF5 format.
  if (!file_exists(NUC_DATA_PATH))
    throw FileNotFound(NUC_DATA_PATH);

  bool ish5 = H5Fis_hdf5(NUC_DATA_PATH.c_str());
  if (!ish5)
    throw h5wrap::FileNotHDF5(NUC_DATA_PATH);

  using rxname::id;
  std::map<unsigned int, size_t> rxns;
  rxns[id("tot")] = offsetof(simple_xs_struct, sigma_t);
  rxns[id("scat")] = offsetof(simple_xs_struct, sigma_s);
  rxns[id("elas")] = offsetof(simple_xs_struct, sigma_e);
  rxns[id("inel")] = offsetof(simple_xs_struct, sigma_i);
  rxns[id("abs")] = offsetof(simple_xs_struct, sigma_a);
  rxns[id("gamma")] = offsetof(simple_xs_struct, sigma_gamma);
  rxns[id("fiss")] = offsetof(simple_xs_struct, sigma_f);
  rxns[id("alpha")] = offsetof(simple_xs_struct, sigma_alpha);
  rxns[id("proton")] = offsetof(simple_xs_struct, sigma_proton);
  rxns[id("deut")] = offsetof(simple_xs_struct, sigma_deut);
  rxns[id("trit")] = offsetof(simple_xs_struct, sigma_trit);
  rxns[id("z_2n")] = offsetof(simple_xs_struct, sigma_2n);
  rxns[id("z_3n")] = offsetof(simple_xs_struct, sigma_3n);
  rxns[id("z_4n")] = offsetof(simple_xs_struct, sigma_4n);

  // Get the HDF5 compound type (table) description
  hid_t desc = H5Tcreate(H5T_COMPOUND, sizeof(simple_xs_struct));
  H5Tinsert(desc, "nuc",   HOFFSET(simple_xs_struct, nuc),   H5T_NATIVE_INT);
  H5Tinsert(desc, "sigma_t",  HOFFSET(simple_xs_struct, sigma_t),  H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_s", HOFFSET(simple_xs_struct, sigma_s), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_e", HOFFSET(simple_xs_struct, sigma_e), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_i", HOFFSET(simple_xs_struct, sigma_i), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_a", HOFFSET(simple_xs_struct, sigma_a), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_gamma", HOFFSET(simple_xs_struct, sigma_gamma), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_f", HOFFSET(simple_xs_struct, sigma_f), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_alpha", HOFFSET(simple_xs_struct, sigma_alpha), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_proton", HOFFSET(simple_xs_struct, sigma_proton), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_deut", HOFFSET(simple_xs_struct, sigma_deut), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_trit", HOFFSET(simple_xs_struct, sigma_trit), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_2n", HOFFSET(simple_xs_struct, sigma_2n), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_3n", HOFFSET(simple_xs_struct, sigma_3n), H5T_NATIVE_DOUBLE);
  H5Tinsert(desc, "sigma_4n", HOFFSET(simple_xs_struct, sigma_4n), H5T_NATIVE_DOUBLE);

  // Open the HDF5 file
  hid_t nuc_data_h5 = H5Fopen(NUC_DATA_PATH.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

  // build path to prober simple xs table
  std::string path = "/neutron/simple_xs/" + energy;

  // Open the data set
  hid_t simple_xs_set = H5Dopen2(nuc_data_h5, path.c_str(), H5P_DEFAULT);
  hid_t simple_xs_space = H5Dget_space(simple_xs_set);
  int n = H5Sget_simple_extent_npoints(simple_xs_space);

  // Read in the data
  simple_xs_struct* array = new simple_xs_struct[n];
  H5Dread(simple_xs_set, desc, H5S_ALL, H5S_ALL, H5P_DEFAULT, array);

  // close the nuc_data library, before doing anything stupid
  H5Dclose(simple_xs_set);
  H5Fclose(nuc_data_h5);

  // Ok now that we have the array of stucts, put it in the map
  for(int i = 0; i < n; i++) {
    std::map<unsigned int, size_t>::iterator it;
    for (it = rxns.begin(); it != rxns.end(); ++it) {
      double xs = *(double*)((char*)&array[i] + it->second);
      simple_xs_map[energy][array[i].nuc][it->first] = xs;
    }
  }
  delete[] array;
}

double simple_xs(int nuc, int rx_id, std::string energy) {
  std::set<std::string> energies;
  energies.insert("thermal");
  energies.insert("thermal_maxwell_ave");
  energies.insert("resonance_integral");
  energies.insert("fourteen_MeV");
  energies.insert("fission_spectrum_ave");

  if (energies.count(energy) == 0) {
    throw InvalidSimpleXS("Energy '" + energy + 
        "' is not a valid simple_xs group");
  } else if (simple_xs_map.count(energy) == 0) {
    _load_simple_xs_map(energy);
  }

  if (simple_xs_map[energy].count(nuc) == 0) {
    throw InvalidSimpleXS(rxname::name(rx_id) + 
        " is not a valid simple_xs nuclide");
  } else if (simple_xs_map[energy][nuc].count(rx_id) == 0) {
    throw InvalidSimpleXS(rxname::name(rx_id) + 
        " is not a valid simple_xs reaction");
  }

  return simple_xs_map[energy][nuc][rx_id];
}

double simple_xs(int nuc, std::string rx, std::string energy) {
  return simple_xs(nucname::id(nuc), rxname::id(rx), energy);
}
double simple_xs(std::string nuc, int rx, std::string energy) {
  return simple_xs(nucname::id(nuc), rxname::id(rx), energy);
}
double simple_xs(std::string nuc, std::string rx, std::string energy) {
  return simple_xs(nucname::id(nuc), rxname::id(rx), energy);
}

} // namespace pyne
