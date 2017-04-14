
# PlanAhead Launch Script for Post-Synthesis floorplanning, created by Project Navigator

create_project -name axi_pmic -dir "U:/Documents/cpre488AudioTracking/system/pcores/axi_pmic_v1_00_a/devl/projnav/planAhead_run_1" -part xc7z020clg484-1
set_property design_mode GateLvl [get_property srcset [current_run -impl]]
set_property edif_top_file "U:/Documents/cpre488AudioTracking/system/pcores/axi_pmic_v1_00_a/devl/projnav/axi_pmic.ngc" [ get_property srcset [ current_run ] ]
add_files -norecurse { {U:/Documents/cpre488AudioTracking/system/pcores/axi_pmic_v1_00_a/devl/projnav} }
set_property target_constrs_file "axi_pmic.ucf" [current_fileset -constrset]
add_files [list {axi_pmic.ucf}] -fileset [get_property constrset [current_run]]
link_design
