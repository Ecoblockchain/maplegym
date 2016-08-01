import maplegym

r_actions = {y: x for x, y in maplegym.possible_actions.iteritems()}

for s, a, r, s2 in maplegym.recorder.record():
    if (a > 0) or (r > 0):
        print 'experience: action = %s, reward = %d' % (r_actions[a], r)
