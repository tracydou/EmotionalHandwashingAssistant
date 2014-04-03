import sys
#import cOptionsGeneral
import cOptions3D
import cOptions2D
from cBayesactSimBuffer import cBayesactSimBuffer
import subprocess


def main(argv):
    #cOptionsGeneral.run(argv)
    #cOptions3D.run(argv)
    cOptions2D.run(argv)

if __name__ == "__main__":
    main(sys.argv[1:])
