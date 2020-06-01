
.PHONY : all
all : 
	@echo "begin to make."; \
	$(MAKE) -C ./src; \
	$(MAKE) -C ./examples; \

# 通配符可以出现在模式的命令中
.PHONY : clean
clean : 
	@echo "clean all object."; \
	$(MAKE) clean -C ./src; \
	$(MAKE) clean -C ./examples; \
