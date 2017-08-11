_level = []

def do(*args, **kwargs):
    print("  " * len(_level), end = "")
    print(*args, **kwargs)

def enter(level):
    do("Begin %s" % (level))
    _level.append(level)

def leave():
    level = _level[-1]
    del _level[-1]
    do("End %s" % (level))
