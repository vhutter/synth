import classes
import sys

def createHeader(path, name):
  f = open(path+name+'.h', 'w')
  guard = name.upper() + '_H_INCLUDED'
  f.write('#ifndef ' + guard + '\n#define ' + guard + '\n\n#include "GuiElement.h"\n\n'+ 'class ' + name + ' : public GuiElement\n{\n\n};\n\n#endif //' + guard)

def createCpp(path, name):
  f = open(path+name+'.cpp', 'w')
  f.write('#include "'+name+'.h"\n\n')

def createClass(path, name):
	createHeader(path, name)
	createCpp   (path, name)
	
if len(sys.argv) < 2:
	exit()
else:
	createClass(
		'../gui/',
		sys.argv[1]
	)