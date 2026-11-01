# Naming convention of the generated operators, shared by generateWrappers.py (which
# writes the wrappers and their tables) and generateSelection.py (which selects the tables)

def operatorPrefix(backend="verrou", rounding=None, post="", soft=False, unfused=False, conv=False):
    res="vr_"
    if unfused:
        res+="unfused_"
    if conv:
        res+="conv_"
    res+=backend
    if rounding!=None:
        res+="_"+rounding
    res+=post
    if soft:
        res+="_soft"
    return res
