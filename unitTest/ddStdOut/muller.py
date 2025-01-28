#!/usr/bin/python3



def muller(nt,verbose=False):
    x0 = 11./2.;
    x1 = 61./11.;

    
    for it in range(nt):
        temp0 = 3000./x0;
        temp1 = 1130. - temp0;
        temp2 = temp1 /x1 ;

        x2 = 111. - temp2;

        if verbose:
            print("it: %i\tx2: %f\ttemp0: %f\ttemp1: %f\ttemp2: %f"%(it,x2,temp0,temp1,temp2))
            

        x0 = x1;
        x1 = x2;
    print("x[%i]=%f"%(nt,x1))
    print("Function object at 0x49996a00>")
    a=10.*4.
    print("a=",a)
    

if __name__=="__main__":
    muller(12, True)
