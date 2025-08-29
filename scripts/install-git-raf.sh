#!/bin/bash
# Git-RAF installation script

echo "Installing Git-RAF Auto-Tag System..."

# Check dependencies
if ! command -v bc &> /dev/null; then
    echo "Installing bc..."
    sudo apt-get update && sudo apt-get install -y bc
fi

if ! command -v openssl &> /dev/null; then
    echo "Installing openssl..."
    sudo apt-get update && sudo apt-get install -y openssl
fi

# Copy to executable location
sudo cp git-raf /usr/local/bin/
sudo chmod +x /usr/local/bin/git-raf

# Initialize configuration
git-raf --init-config

# Install hooks
git-raf --install-hooks

echo "Git-RAF installed successfully."
echo "Run 'make release' followed by 'git-raf --tag' to create your first tag."
