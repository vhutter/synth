import classes

def createHeader(path, name):
  f = open(path+name+'.h', 'w')
  guard = name.upper() + '_H_INCLUDED'
  f.write('#ifndef ' + guard + '\n#define ' + guard + '\n\n#include "../guiElements.h"\n\n'+ 'class ' + name + ' : public GuiElement\n{\n\n};\n\n#endif //' + guard)