# Source this file: . tools/session/env.sh
# GW: repository root. S: scratch directory (ignored by git). B: the base ROM.
_here=$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)
export GW=${GW:-$(cd "$_here/../.." && pwd)}
export S=${S:-$GW/.session}
export B=${B:-${BASEROM:-$GW/baserom.us.z64}}
export BASEROM=$B
export JOBS=${JOBS:-$(nproc 2>/dev/null || echo 2)}
mkdir -p "$S"
[ -d "$GW/.venv/bin" ] && export PATH="$GW/.venv/bin:$PATH"
export PATH="$_here:$PATH"
unset _here
