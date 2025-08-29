# Git-RAF Auto-Tag System Integration
.PHONY: release-tag
release-tag: release
	@echo "Creating stable release tag..."
	@git-raf --tag || echo "Build not stable enough for tagging"

.PHONY: verify-build
verify-build:
	@git-raf --verify

.PHONY: sinphase
sinphase:
	@git-raf --sinphase

.PHONY: install-git-raf
install-git-raf:
	@sudo cp tools/git-raf /usr/local/bin/
	@sudo chmod +x /usr/local/bin/git-raf
	@git-raf --init-config
	@git-raf --install-hooks
	@echo "Git-RAF installed successfully."
