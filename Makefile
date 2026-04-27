CXX = g++
CXXFLAGS =  -std=c++23 -O3 -march=native
TARGET = practice5_DAA

SOURCES =  src/main.cc src/solutions/ms_cflp_ci_solution.cc src/instances/ms_cflp_ci_instance.cc src/instances/ms_cflp_ci_instance_csi_loader.cc src/algorythms/ms_cflp_ci_general_solver.cc src/algorythms/greedy_ms_cflp_ci_solver.cc src/algorythms/grasp_ms_cflp_ci_solver.cc src/algorythms/grasp_algorythm.cc src/explorers/ms_cflp_ci_shift_explorer.cc src/explorers/ms_cflp_ci_swap_explorer.cc src/explorers/ms_cflp_ci_facility_swap_explorer.cc src/explorers/ms_cflp_ci_incompatibilities_remover_explorer.cc src/algorythms/grasp_ms_cflp_ci_vnd_solver.cc src/algorythms/grasp_ms_cflp_ci_rvnd_solver.cc src/perturbators/ms_cflp_ci_shift_perturbator.cc src/algorythms/grasp_ms_cflp_ci_gvns_rl_solver.cc src/solutions/ms_cflp_ci_solution_saver.cc

OBJECTS = $(SOURCES:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

cleano:
	rm -f $(OBJECTS)

.PHONY: all clean cleano