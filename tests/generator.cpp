// test data generator for modding suite,
// creates dat files with some random ndfbin data
// and their corresponding xml files

#include <argparse/argparse.hpp>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program;
  program.add_argument("-o").help(gettext("Path to output folder"));

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }
}
