import classes

def createCpp(path, name):
  f = open(path+name+'.cpp', 'w')
  f.write('#include "'+name+'.h"\n\n')