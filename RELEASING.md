# Creating a New Release

Follow these steps each time you ship a new version.

## 1. Decide the version number

Use [Semantic Versioning](https://semver.org):

| Change type | Example bump |
|-------------|-------------|
| Bug fix, patch | `2.1.0` → `2.1.1` |
| New backwards-compatible feature | `2.1.0` → `2.2.0` |
| Breaking change | `2.1.0` → `3.0.0` |

## 2. Update `package.json`

Edit the `"version"` field:

```json
{
  "version": "2.2.0"
}
```

## 3. Update `CHANGELOG.md`

Add a new section **at the top of the release list**:

```markdown
## [2.2.0] - YYYY-MM-DD

### Added
- …

### Changed
- …

### Fixed
- …

### Removed
- …

[2.2.0]: https://github.com/howardginsburg/framework-arduinostm32mxchip/releases/tag/v2.2.0
```

Only include the headings that apply. Common headings are `Added`, `Changed`, `Fixed`, `Removed`, `Security`, and `Deprecated`.

## 4. Commit, tag, and push

```powershell
git add package.json CHANGELOG.md
git commit -m "Release v2.2.0"
git tag v2.2.0
git push origin main --tags
```

## 5. Create a GitHub Release

1. Go to [Releases](https://github.com/howardginsburg/framework-arduinostm32mxchip/releases) and click **Draft a new release**.
2. Choose the tag you just pushed (`v2.2.0`).
3. Set the title to `v2.2.0`.
4. Paste the changelog entry for this version into the description.
5. Click **Publish release**.

GitHub automatically generates a `.zip` archive for the tag, which is what users reference in their `platformio.ini`.
