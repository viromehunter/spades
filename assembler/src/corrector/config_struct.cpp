#include "config_struct.hpp"

#include "openmp_wrapper.h"

#include <yaml-cpp/yaml.h>
#include <string>



namespace corrector {
void load(corrector_config& cfg, const std::string &filename) {
  YAML::Node config = YAML::LoadFile(filename);

  cfg.dataset.load(config["dataset"].as<std::string>());

  cfg.working_dir = config["working_dir"].as<std::string>(".");
  cfg.output_dir = config["output_dir"].as<std::string>(".");

  // FIXME: Make trivial deserialization trivial
  cfg.max_nthreads = config["max_nthreads"].as<unsigned>();
  // Fix number of threads according to OMP capabilities.
  cfg.max_nthreads = std::min(cfg.max_nthreads, (unsigned)omp_get_max_threads());
  // Inform OpenMP runtime about this :)
  omp_set_num_threads(cfg.max_nthreads);
}
}
