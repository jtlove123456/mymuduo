# 定义参与编译的源代码
aux_source_directory(./base SRC_BASE_LIST)
aux_source_directory(./net SRC_NET_LIST)
# 设置编译生成动态库SHARED 
add_library(mymuduo SHARED ${SRC_BASE_LIST} ${SRC_NET_LIST})

# # 设置编译生成可执行文件
# add_executable(test ${SRC_BASE_LIST} ${SRC_NET_LIST})

# 修改库名称properties
# set_target_properties(server PROPERTIES OUTPUT_NAME "libserver")



