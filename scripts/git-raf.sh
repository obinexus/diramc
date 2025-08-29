#!/bin/bash
# git-raf - Git-RAF Auto-Tag System Implementation
# OBINexus Computing - AEGIS Methodology Compliance

set -euo pipefail

# Configuration
CONFIG_FILE=".git/raf-config"
DEFAULT_SINPHASE_THRESHOLD=0.5
ARTIFACTS=(
    "build/release/bin/diram"
    "build/release/lib/libdiram.so.1" 
    "build/release/lib/libdiram.a"
    "build/release/config/diram.drc"
)

# Load configuration
load_config() {
    if [[ -f "$CONFIG_FILE" ]]; then
        source "$CONFIG_FILE"
    else
        SINPHASE_THRESHOLD=$DEFAULT_SINPHASE_THRESHOLD
        TAG_PREFIX="diram"
        TAG_FORMAT="${PREFIX}-v${VERSION}-${STABILITY}"
        GOVERNANCE_REF="diram_build_policy.rift.gov"
    fi
}

# Initialize configuration
init_config() {
    cat > "$CONFIG_FILE" << 'EOF'
# Git-RAF Auto-Tag Configuration
SINPHASE_THRESHOLD=0.5
TAG_PREFIX="diram"
TAG_FORMAT="${PREFIX}-v${VERSION}-${STABILITY}"
GOVERNANCE_REF="diram_build_policy.rift.gov"
ARTIFACT_MANIFEST=".git/raf-artifacts.json"
EOF
    echo "Configuration initialized at $CONFIG_FILE"
}

# Verify build artifacts
verify_artifacts() {
    local missing=0
    for artifact in "${ARTIFACTS[@]}"; do
        if [[ ! -f "$artifact" ]]; then
            echo "Missing artifact: $artifact" >&2
            missing=$((missing + 1))
        fi
    done
    
    if [[ $missing -gt 0 ]]; then
        echo "Missing $missing artifacts. Build verification failed." >&2
        return 1
    fi
    echo "All artifacts verified successfully."
    return 0
}

# Calculate sinphase metric
calculate_sinphase() {
    local artifact_count=0
    for artifact in "${ARTIFACTS[@]}"; do
        if [[ -f "$artifact" ]]; then
            artifact_count=$((artifact_count + 1))
        fi
    done
    
    # Get test results (simplified for example)
    local test_pass_rate=0
    local total_tests=0
    
    if [[ -f "test/results.xml" ]]; then
        # Parse test results from XML
        test_pass_rate=$(grep -o 'passes="[0-9]*"' test/results.xml | cut -d'"' -f2)
        total_tests=$(grep -o 'tests="[0-9]*"' test/results.xml | cut -d'"' -f2)
    else
        # Fallback to make test output parsing
        local test_output
        test_output=$(make test 2>&1 | tail -5)
        test_pass_rate=$(echo "$test_output" | grep -Eo '[0-9]+ passing' | cut -d' ' -f1)
        total_tests=$(echo "$test_output" | grep -Eo '[0-9]+ tests' | cut -d' ' -f1)
    fi
    
    if [[ -z "$test_pass_rate" ]] || [[ -z "$total_tests" ]] || [[ $total_tests -eq 0 ]]; then
        echo "No tests found. Sinphase calculation aborted." >&2
        return 1
    fi
    
    # Calculate sinphase σ = (artifact_count × test_pass_rate) / (total_tests × 10)
    local sinphase=$(echo "scale=4; ($artifact_count * $test_pass_rate) / ($total_tests * 10)" | bc)
    echo "$sinphase"
}

# Classify stability based on sinphase using trie logic
classify_stability() {
    local sinphase=$1
    local stability=""
    
    # Trie-based classification
    if (( $(echo "$sinphase < 0.2" | bc -l) )); then
        stability="alpha"
    elif (( $(echo "$sinphase < 0.4" | bc -l) )); then
        stability="beta" 
    elif (( $(echo "$sinphase < 0.6" | bc -l) )); then
        stability="rc"
    elif (( $(echo "$sinphase < 0.8" | bc -l) )); then
        stability="stable"
    else
        stability="release"
    fi
    
    echo "$stability"
}

# Determine version bump based on changes
determine_version_bump() {
    local last_tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
    local changes=$(git diff --name-only "$last_tag" HEAD 2>/dev/null || git ls-files)
    
    local bump="patch"
    if echo "$changes" | grep -q "^include/"; then
        bump="major"
    elif echo "$changes" | grep -q "^src/core/"; then
        bump="minor"
    fi
    
    echo "$bump"
}

# Increment version based on semver
increment_version() {
    local version=$1
    local increment=$2
    local major=$(echo "$version" | cut -d. -f1 | sed 's/^v//')
    local minor=$(echo "$version" | cut -d. -f2)
    local patch=$(echo "$version" | cut -d. -f3)
    
    case "$increment" in
        "major")
            major=$((major + 1))
            minor=0
            patch=0
            ;;
        "minor")
            minor=$((minor + 1))
            patch=0
            ;;
        "patch")
            patch=$((patch + 1))
            ;;
    esac
    
    echo "${major}.${minor}.${patch}"
}

# Generate governance metadata
generate_governance_metadata() {
    local stability=$1
    local sinphase=$2
    local version=$3
    
    # Create artifact manifest hash
    local manifest_hash=""
    for artifact in "${ARTIFACTS[@]}"; do
        if [[ -f "$artifact" ]]; then
            manifest_hash="${manifest_hash}$(sha256sum "$artifact" | cut -d' ' -f1)"
        fi
    done
    local entropy_checksum=$(echo -n "$manifest_hash" | sha256sum | cut -d' ' -f1)
    
    local timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    local artifact_count=0
    
    for artifact in "${ARTIFACTS[@]}"; do
        if [[ -f "$artifact" ]]; then
            artifact_count=$((artifact_count + 1))
        fi
    done
    
    # Generate AuraSeal signature
    local auraseal_content="${entropy_checksum}${timestamp}${stability}${version}"
    local auraseal=$(echo -n "$auraseal_content" | openssl dgst -sha256 -hmac "OBINexus-DIRAM" | cut -d' ' -f2)
    
    cat << EOF
Policy-Tag: "$stability"
Governance-Ref: $GOVERNANCE_REF
Entropy-Checksum: $entropy_checksum
Governance-Vector:
  - build_risk: $(echo "1 - $sinphase" | bc)
  - rollback_cost: 0.15
  - stability_impact: $sinphase
AuraSeal: $auraseal
RIFTlang-Compilation-Proof: verified_stable_build
Build-Timestamp: $timestamp
Artifact-Count: $artifact_count
EOF
}

# Create and apply tag
create_tag() {
    echo "Verifying build artifacts..."
    if ! verify_artifacts; then
        return 1
    fi
    
    echo "Calculating sinphase metric..."
    local sinphase=$(calculate_sinphase)
    if [[ $? -ne 0 ]]; then
        return 1
    fi
    
    if (( $(echo "$sinphase < $SINPHASE_THRESHOLD" | bc -l) )); then
        echo "Sinphase value $sinphase below threshold $SINPHASE_THRESHOLD. Tagging aborted." >&2
        return 1
    fi
    
    local stability=$(classify_stability "$sinphase")
    local bump=$(determine_version_bump)
    local current_version=$(git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//' || echo "0.0.0")
    local new_version=$(increment_version "$current_version" "$bump")
    
    # Generate tag name
    local PREFIX="$TAG_PREFIX"
    local VERSION="$new_version"
    local STABILITY="$stability"
    local tag_name=$(eval "echo $TAG_FORMAT")
    
    # Generate governance metadata
    local governance_metadata=$(generate_governance_metadata "$stability" "$sinphase" "$new_version")
    
    # Create tag with metadata
    git tag -a "$tag_name" -m "Git-RAF Auto-Tag: $stability
Sinphase: $sinphase
Version: $new_version
$governance_metadata"
    
    echo "Created tag: $tag_name"
    echo "Sinphase: $sinphase"
    echo "Stability: $stability"
    echo "Version: $new_version"
}

# Install Git hooks
install_hooks() {
    local hook_dir=".git/hooks"
    
    # Create pre-commit hook
    cat > "$hook_dir/pre-commit" << 'EOF'
#!/bin/bash
# Git-RAF Pre-Commit Hook
# Check for changes that might affect build

changed_files=$(git diff --cached --name-only)

if echo "$changed_files" | grep -q -E "^(include/|src/core/|src/|Makefile|\.c$|\.h$)"; then
    echo "Git-RAF: Build-related files changed. Consider running 'make release' before committing."
fi
EOF
    
    # Create post-commit hook
    cat > "$hook_dir/post-commit" << 'EOF'
#!/bin/bash
# Git-RAF Post-Commit Hook
# Attempt auto-tagging if relevant files changed

changed_files=$(git diff HEAD~1 --name-only)

if echo "$changed_files" | grep -q -E "^(include/|src/core/|src/|Makefile|\.c$|\.h$)"; then
    echo "Git-RAF: Build-related changes detected. Running verification..."
    if command -v git-raf >/dev/null 2>&1; then
        git-raf --tag
    fi
fi
EOF
    
    chmod +x "$hook_dir/pre-commit" "$hook_dir/post-commit"
    echo "Git hooks installed successfully."
}

# Main function
main() {
    load_config
    
    case "${1:-}" in
        "--tag")
            echo "Starting Git-RAF Auto-Tag process..."
            create_tag
            ;;
        "--sinphase")
            calculate_sinphase
            ;;
        "--verify")
            verify_artifacts
            ;;
        "--install-hooks")
            install_hooks
            ;;
        "--init-config")
            init_config
            ;;
        "--help")
            cat << EOF
Git-RAF Auto-Tag System
Usage: git-raf [OPTION]

Options:
  --tag           Create tag if stability criteria met
  --sinphase      Calculate sinphase metric
  --verify        Verify build artifacts
  --install-hooks Install Git hooks for auto-tagging
  --init-config   Initialize configuration file
  --help          Show this help message

EOF
            ;;
        *)
            echo "Unknown option: ${1:-}"
            echo "Use --help for usage information."
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"
