cc = bcc32
opt = -W -w-
obj = main.obj cmnctrl.obj cmdproc.obj storectrl.obj csvreader.obj
exe = Release\FLE@M

.cpp.obj:
	$(cc) $(opt) -c $<

$(exe).exe: $(obj)
	$(cc) $(opt) -e$* $**

main.obj: main.cpp
cmnctrl.obj: cmnctrl.cpp
cmdproc.obj: cmdproc.cpp
storectrl.obj: storectrl.cpp
csvreader.obj: csvreader.cpp