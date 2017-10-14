# Author: Dominik Harmim <xharmi00@stud.fit.vutbr.cz>

PACK = xharmi00.zip


.PHONY: pack
pack: $(PACK)


$(PACK): # TODO - files to pack
	make clean
	zip $@ $^


.PHONY: clean
clean:
	rm -rf $(PACK) \
		Debug/*.exe Debug/*.ilk Debug/*.pdb Debug/*.exp Debug/*.lib Debug/Student_DLL.dll \
		Image_editor/Release/ Image_editor/Debug/ \
		Student_DLL/Debug/ Student_DLL/Release/


