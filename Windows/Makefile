CFLAGS = /nologo /DDLL_EXPORTS /c

build: so_stdio.dll

so_stdio.dll: so_stdio.obj
	link /nologo /dll /out:$@ /implib:so_stdio.lib $**

so_stdio.obj: so_stdio.c
	cl $(CFLAGS) /Fo$@ $**

clean:
	del *.obj so_stdio.dll

