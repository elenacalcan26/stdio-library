build: so_stdio.dll

so_stdio.dll: so_stdio.obj
	link /nologo /dll /out:$@ /implib:so_stdio.lib $**

so_stdio.obj: so_stdio.c
	cl /MD /c /Fo$@ $**

clean:
	del *.obj so_stdio.dll

