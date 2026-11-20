#!/usr/bin/env python3
"""enumify.py: replace numeric constants with the names in include/enums.h, where the context says which
enum the value belongs to (for example a comparison with hand.state->mode, the first argument of ApplyAction,
an index into pad[].button, or a case of a switch on an NMEInstruction's type).

    python3 tools/session/enumify.py [--dry] [ENUM ...]   (default: every rule)

Uses the constants recorded by xref.py. A literal is replaced only when the source text at its recorded
position is that literal. Constants that come from macros are left alone. Run bld afterwards.
"""
import collections, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
GW = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
import xref  # noqa: E402

CMP = r'(cmp==|cmp!=|set|case)'
# (enum, kind regex, where regex, function regex or None, struct field or None)
RULES = [
    ('HandAction', CMP, r'hand\.state->(mode|prevMode)|handControl\.prevAction', None, None),
    ('HandAction', CMP, r'mode', r'IsHandHoldingBall', None),
    ('HandAction', r'case', r'action|requestedAction', r'ApplyAction|ConditionsMet', None),
    ('HandAction', r'arg0', r'(ApplyAction|ConditionsMet)\|.*', None, None),
    ('HandAnim', r'arg1', r'AnimateActor\|&hand', None, None),
    ('HandAnim', CMP, r'hand\.anim->current', None, None),
    ('HandWalk', CMP, r'handControl\.(gait|prevGait)', None, None),
    ('HandStatus', CMP, r'handControl\.status', None, None),
    ('HurtType', r'arg0', r'(KillHand|HurtHand)\|.*', None, None),
    ('InWater', CMP, r'.*waterState', None, None),
    ('Button', r'index', r'pad\[[^\]]*\]\.(button|pressed|prevButton|holdTime|sincePress|latch)', None, None),
    ('ControlMethod', CMP, r'controlMethod', None, None),
    ('BallMode', CMP, r'ballControl\.type', None, None),
    ('BallState', CMP, r'ballControl\.state', None, None),
    ('Pickup', r'index', r'player\.powerups', None, None),
    ('Pickup', r'arg0', r'(ActivateCollectable|DeactivateCollectable)\|.*', None, None),
    ('Difficulty', CMP, r'saveSlot\[[^\]]*\]\.difficultyLevel', None, None),
    ('World', CMP, r'gameInfo\.world', None, None),
    ('Level', CMP, r'gameInfo\.level', None, None),
    ('Level', r'arg0', r'StartNewLevel\|.*', None, None),
    ('ExitType', CMP, r'gameInfo\.exitType', None, None),
    ('ExitType', r'arg0', r'ExitLevel\|.*', None, None),
    ('Track', r'arg0', r'PrepareSong\|.*', None, None),
    ('DebrisType', r'arg0', r'AddDebris\|.*', None, None),
    ('NmeInstruction', CMP, r'.*\btype', None, ('NMEInstruction', 'type')),
    ('NmeCondition', CMP, r'.*\bcondition', None, ('NMEInstruction', 'condition')),
    ('PuzzleActionType', CMP, r'.*\btype', None, ('PuzzleAction', 'type')),
    ('PuzzleCondType', CMP, r'.*\btype', None, ('PuzzleCond', 'type')),
    ('ActorType', CMP, r'.*\btype', None, ('Actor', 'type')),
    ('Token', CMP, r'token', None, None),
    ('PuzzleActionType', CMP, r'action->type', r'PuzzleExecuteAction', None),
    ('PuzzleCondType', CMP, r'condition->type', r'PuzzleEvaluateCondition', None),
    ('NmeInstruction', CMP, r'ins->type', r'NMEProcessInstruction', None),
    ('NmeCondition', CMP, r'instruction->condition', r'NMEConditionsMet', None),
    ('CameoInstruction', CMP, r'instruction->type', r'ProcessCameoInstruction', None),
    ('BulletType', CMP, r'bullet->type', None, None),
    ('BulletType', CMP, r'.*\btype', None, ('Bullet', 'type')),
    ('BulletType', r'arg0', r'CreateNewBullet\|.*', None, None),
    ('BulletType', r'arg1', r'FireBulletFromActor\|.*', None, None),
    ('BulletType', CMP, r'.*\bbulletType', None, ('Vent', 'bulletType')),
    ('VentType', CMP, r'.*\btype', None, ('Vent', 'type')),
    ('GaribType', CMP, r'garib->type', None, None),
    ('TexBank', r'arg0', r'LoadTextureBank\|.*', None, None),
    ('TexBank', CMP, r'num', r'LoadTextureBank', None),
    ('Level', CMP, r'num', r'LoadLevel', None),
    ('ScreenDebrisType', r'arg4', r'AddScreenDebris\|.*', None, None),
    ('ScreenDebrisType', r'arg3', r'AddStarBurst\|.*', None, None),
    ('ScreenDebrisType', CMP, r'type', r'AddScreenDebris', None),
]


def load_enums():
    text = open(os.path.join(GW, 'include', 'enums.h')).read()
    enums = {}
    for name, body in re.findall(r'enum (\w+) \{(.*?)\};', text, re.S):
        v, names = -1, {}
        for item in re.findall(r'(\w+)(?:\s*=\s*(-?\w+))?\s*,', body):
            v = int(item[1], 0) if item[1] else v + 1
            if not item[0].startswith(('NUM_', 'MAX_')) and v not in names:
                names[v] = item[0]
        enums[name] = names
    return enums


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    dry = '--dry' in sys.argv
    enums = load_enums()
    xref.build()
    d = xref.load()
    fields = collections.defaultdict(set)  # (file, line) -> {(struct, field)}
    for a in d['accesses']:
        fields[(a['file'], a['line'])].add((a['struct'], a['field']))
    edits = collections.defaultdict(list)  # file -> [(line, col, literal, name, enum)]
    counts = collections.Counter()
    for c in d['consts']:
        kind = c['kind'] if c['kind'].startswith(('cmp', 'case', 'set', 'index', 'return')) else c['kind'][:4] + c['kind'][4:]
        for enum, kre, wre, fre, sf in RULES:
            if args and enum not in args:
                continue
            if not re.fullmatch(kre, kind) or not re.fullmatch(wre, c['where']):
                continue
            if fre and not re.fullmatch(fre, c['func'] or ''):
                continue
            if sf and sf not in fields[(c['file'], c['line'])]:
                continue
            try:
                v = int(c['text'].rstrip('uUlL'), 0)
            except ValueError:
                break
            if v in enums[enum]:
                edits[c['file']].append((c['line'], c['col'], c['text'], enums[enum][v], enum))
            break
    skipped = 0
    for f, es in edits.items():
        path = os.path.join(GW, f)
        lines = open(path).read().split('\n')
        for ln, col, lit, name, enum in sorted(es, key=lambda e: (e[0], -e[1])):
            text = lines[ln - 1]
            i = col - 1
            if text[i:i + len(lit)] != lit or re.match(r'\w', text[i + len(lit):i + len(lit) + 1] or ' ') \
                    or (i and re.match(r'[\w.]', text[i - 1])) or '...' in text:
                skipped += 1
                continue
            lines[ln - 1] = text[:i] + name + text[i + len(lit):]
            counts[enum] += 1
        if not dry:
            open(path, 'w').write('\n'.join(lines))
    for k, v in counts.most_common():
        print(f'{v:5d} {k}')
    print(f'{sum(counts.values())} replaced, {skipped} skipped (text did not match, macro or range)')


if __name__ == '__main__':
    main()
