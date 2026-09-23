#!/usr/bin/env bash
# Command line level sanity checks: separate process executions, file mode and
# binary input.  Usage:  tests/run_sanity.sh [path-to-build-directory]
set -u

build_dir="${1:-build}"
binary="${build_dir}/hash-generator"

if [[ ! -x "${binary}" ]]; then
  echo "error: ${binary} not found, build the project first" >&2
  exit 1
fi

checks=0
failures=0

run() { "${binary}" "$@" 2>/dev/null; }

check() {
  local condition="$1" description="$2"
  checks=$((checks + 1))
  if [[ "${condition}" == "yes" ]]; then
    echo "ok    ${description}"
  else
    failures=$((failures + 1))
    echo "FAIL  ${description}"
  fi
}

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT

# --- output shape -----------------------------------------------------------
for text in "" "a" "b" "hello" "Hello" "abc" "cba"; do
  digest="$(run --text "${text}")"
  if [[ ${#digest} -eq 64 && "${digest}" =~ ^[0-9a-f]{64}$ ]]; then
    check yes "--text \"${text}\" prints 64 lowercase hex characters"
  else
    check no "--text \"${text}\" prints 64 lowercase hex characters (got: ${digest})"
  fi
done

# --- determinism across separate program executions -------------------------
a_first="$(run --text "A")"
b_only="$(run --text "B")"
a_second="$(run --text "A")"
[[ "${a_first}" == "${a_second}" ]] && check yes "A is identical in two separate executions" \
                                    || check no  "A is identical in two separate executions"
[[ "${a_first}" != "${b_only}" ]] && check yes "A and B differ" || check no "A and B differ"

# --- input sensitivity ------------------------------------------------------
[[ "$(run --text 'hello')" != "$(run --text 'Hello')" ]] \
  && check yes "hello differs from Hello" || check no "hello differs from Hello"
[[ "$(run --text 'abc')" != "$(run --text 'cba')" ]] \
  && check yes "abc differs from cba" || check no "abc differs from cba"
printf 'hello\n' > "${work_dir}/with_newline"
printf 'hello'   > "${work_dir}/without_newline"
[[ "$(run --file "${work_dir}/with_newline")" != "$(run --file "${work_dir}/without_newline")" ]] \
  && check yes "hello differs from hello with a trailing newline" \
  || check no  "hello differs from hello with a trailing newline"

# --- file mode hashes the contents, not the name ----------------------------
printf 'hello' > "${work_dir}/name_one"
printf 'hello' > "${work_dir}/completely_different_name"
[[ "$(run --file "${work_dir}/name_one")" == "$(run --file "${work_dir}/completely_different_name")" ]] \
  && check yes "two files with equal contents hash equally" \
  || check no  "two files with equal contents hash equally"
[[ "$(run --file "${work_dir}/without_newline")" == "$(run --text 'hello')" ]] \
  && check yes "file mode and text mode agree on the same bytes" \
  || check no  "file mode and text mode agree on the same bytes"

# --- line endings are not rewritten ----------------------------------------
printf 'line\r\n' > "${work_dir}/crlf"
printf 'line\n'   > "${work_dir}/lf"
[[ "$(run --file "${work_dir}/crlf")" != "$(run --file "${work_dir}/lf")" ]] \
  && check yes "CRLF and LF files hash differently" || check no "CRLF and LF files hash differently"

# --- binary file containing every byte value --------------------------------
binary_file="${work_dir}/all_bytes.bin"
: > "${binary_file}"
for value in $(seq 0 255); do
  printf "$(printf '\\x%02x' "${value}")" >> "${binary_file}"
done
size="$(wc -c < "${binary_file}")"
[[ "${size}" -eq 256 ]] && check yes "binary test file contains 256 bytes" \
                        || check no  "binary test file contains 256 bytes (got ${size})"
binary_digest="$(run --file "${binary_file}")"
[[ "${binary_digest}" =~ ^[0-9a-f]{64}$ ]] && check yes "binary file hashes to 64 lowercase hex characters" \
                                           || check no  "binary file hashes to 64 lowercase hex characters"
[[ "${binary_digest}" == "$(run --file "${binary_file}")" ]] \
  && check yes "binary file digest is stable across executions" \
  || check no  "binary file digest is stable across executions"

# --- empty file -------------------------------------------------------------
: > "${work_dir}/empty"
[[ "$(run --file "${work_dir}/empty")" == "$(run --text '')" ]] \
  && check yes "an empty file matches the empty string" || check no "an empty file matches the empty string"

# --- manual input mode -----------------------------------------------------
typed="$(printf 'hello\n' | "${binary}" 2>/dev/null)"
[[ "${typed}" == "$(run --text 'hello' 2>/dev/null)" ]] \
  && check yes "typed hello + Enter equals --text hello (Enter not included)" \
  || check no  "typed hello + Enter equals --text hello (Enter not included)"
[[ "$(printf 'hello' | "${binary}" 2>/dev/null)" == "${typed}" ]] \
  && check yes "typed input without a final newline is accepted" \
  || check no  "typed input without a final newline is accepted"
[[ "$(printf '\n' | "${binary}" 2>/dev/null)" == "$(run --text '' 2>/dev/null)" ]] \
  && check yes "an empty typed line hashes as empty input" \
  || check no  "an empty typed line hashes as empty input"
[[ "$(printf 'hello\n' | "${binary}" 2>&1 >/dev/null)" == *"Režimas"* ]] \
  && check yes "the active mode is reported on stderr" \
  || check no  "the active mode is reported on stderr"
if printf '' | "${binary}" > /dev/null 2>&1; then
  check no "no typed line at all returns a non-zero exit status"
else
  check yes "no typed line at all returns a non-zero exit status"
fi

# --- read failures are reported, never hashed as empty ----------------------
if "${binary}" --file "${work_dir}/does_not_exist" > "${work_dir}/out" 2> "${work_dir}/err"; then
  check no "a missing file returns a non-zero exit status"
else
  check yes "a missing file returns a non-zero exit status"
fi
[[ -s "${work_dir}/err" && ! -s "${work_dir}/out" ]] \
  && check yes "a missing file prints an error and no digest" \
  || check no  "a missing file prints an error and no digest"
if "${binary}" --file "${work_dir}" > "${work_dir}/out2" 2>/dev/null; then
  check no "a directory argument returns a non-zero exit status"
else
  check yes "a directory argument returns a non-zero exit status"
fi

echo
if [[ "${failures}" -eq 0 ]]; then
  echo "ALL CHECKS PASSED  (${checks}/${checks})"
  exit 0
fi
echo "SOME CHECKS FAILED  ($((checks - failures))/${checks})"
exit 1
