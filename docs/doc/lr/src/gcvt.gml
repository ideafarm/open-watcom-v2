.func gcvt _gcvt _wgcvt
.synop begin
#include <stdlib.h>
char *gcvt( double value,
            int ndigits,
            char *buffer );
.ixfunc2 '&Conversion' &func
.if &'length(&_func.) ne 0 .do begin
char *_gcvt( double value,
            int ndigits,
            char *buffer );
.ixfunc2 '&Conversion' &_func
.do end
.if &'length(&wfunc.) ne 0 .do begin
wchar_t *_wgcvt( double value,
                 int ndigits,
                 wchar_t *buffer );
.ixfunc2 '&Conversion' &wfunc
.do end
.synop end
.desc begin
The &func function converts the floating-point number
.arg value
into a character string and stores the result in
.arg buffer
.ct .li .
The parameter
.arg ndigits
specifies the number of significant digits desired.
The converted number will be rounded to this position.
.np
If the exponent of the number is less than &minus.4 or is greater than
or equal to the number of significant digits wanted, then the number
is converted into E-format, otherwise the number is formatted using
F-format.
.if &'length(&_func.) ne 0 .do begin
.np
The &_func function is identical to &func..
Use &_func for ANSI/ISO naming conventions.
.do end
.if &'length(&wfunc.) ne 0 .do begin
.np
The &wfunc function is identical to &func except that it produces a
wide-character string (which is twice as long).
.do end
.desc end
.return begin
The &func function returns a pointer to the string of digits.
.return end
.see begin
.seelist gcvt ecvt fcvt printf
.see end
.exmp begin
#include <stdio.h>
#include <stdlib.h>

void main()
  {
    char buffer[80];
.exmp break
    printf( "%s\n", gcvt( -123.456789, 5, buffer ) );
    printf( "%s\n", gcvt( 123.456789E+12, 5, buffer ) );
  }
.exmp output
-123.46
1.2346E+014
.exmp end
.class begin WATCOM
.ansiname &_func
.class end
.system
