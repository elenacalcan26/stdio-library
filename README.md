# stdio-library

<h5> Calcan Elena-Claudia <br/>
331CA</h5> 


### Organizare
-----------------------------------------------

* toate functiile au fost implementate in fisierul so_stdio.c
* de asemenea, tot acolo s-a definit structura so_file ce retine urmatoarele informatii: 
  * file descriptor-ul fisierului 
  * un buffer din care se scriu si citesc date, cat si lungimea acestuia si o variabila ce indica pozitia in buffer
  * un cursor al fisierului
  * doua flag-uri de indicare a erorilor
  * ultima operatia facuta asupra fisierului
  * pid-ul procesului copil in care se fac redirectarile  
    
### Implementare 
-----------------------------------------------
* pe Linux este implementata toata functionalitatea temei 
* pe Windows nu este implemntata partea de procese si in plus mai pica niste teste
* in aceasta implementare s-a mai scris o functie care aloca structura si 
 initializeaza campurile cu valori default 
* functiile de so_fread si so_fwrtie sunt implementate pe baza functiilor so_fgetc/so_putc 
* functia de so_fuptc foloseste so_fflush pentru a scrie datele din buffer 
* partea de procese este implementata folosind modelul din laboratorul 3 


### Compilare
--------------------------------------------------
* pentru compilare se foloseste make / nmake 
* in urma comenzilor rezulta biblioteca dinamaica libso_stdio.so / so_stdio.dll

### Bibliografie 
------------------------------------------------ 
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-03
