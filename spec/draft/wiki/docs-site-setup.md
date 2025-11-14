# Static Site Generator Options for XInnoDB Documentation

## Option 1: MkDocs + Material Theme (Recommended for Documentation)

### Setup

```bash
# Install MkDocs
pip install mkdocs mkdocs-material

# Create docs structure
mkdocs new .

# Directory structure
docs/
├── index.md              # Home page
├── architecture.md
├── building.md
├── contributing.md
├── modules.md
└── getting-started.md

mkdocs.yml                # Configuration
```

### Configuration (mkdocs.yml)

```yaml
site_name: XInnoDB Documentation
site_url: https://filasieno.github.io/innodb
repo_url: https://github.com/filasieno/innodb
repo_name: filasieno/innodb

theme:
  name: material
  palette:
    - scheme: default
      primary: indigo
      accent: indigo
  features:
    - navigation.tabs
    - navigation.sections
    - navigation.expand
    - search.suggest
    - search.highlight

nav:
  - Home: index.md
  - Getting Started: getting-started.md
  - Architecture: architecture.md
  - Building: building.md
  - Modules: modules.md
  - Contributing: contributing.md

markdown_extensions:
  - pymdownx.highlight
  - pymdownx.superfences:
      custom_fences:
        - name: mermaid
          class: mermaid
  - admonition
  - pymdownx.details
  - pymdownx.tabbed
  - toc:
      permalink: true

plugins:
  - search
  - git-revision-date-localized
```

### Build and Deploy

```bash
# Build static site
mkdocs build

# Preview locally
mkdocs serve

# Deploy to GitHub Pages
mkdocs gh-deploy
```

### GitHub Actions Auto-Deploy

`.github/workflows/docs.yml`:

```yaml
name: Deploy Documentation
on:
  push:
    branches: [master]
    paths:
      - 'docs/**'
      - 'mkdocs.yml'

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-python@v4
        with:
          python-version: 3.x
      - run: pip install mkdocs-material
      - run: mkdocs gh-deploy --force
```

**Result**: Documentation at `https://filasieno.github.io/innodb`

---

## Option 2: Docusaurus (Modern, Feature-Rich)

### Setup

```bash
# Create Docusaurus site
npx create-docusaurus@latest docs-site classic

cd docs-site

# Directory structure
docs/
├── intro.md
├── architecture.md
├── building.md
└── ...

docusaurus.config.js      # Configuration
```

### Configuration

```javascript
module.exports = {
  title: 'XInnoDB',
  tagline: 'High-performance ABI-stable storage engine',
  url: 'https://filasieno.github.io',
  baseUrl: '/innodb/',
  projectName: 'innodb',
  organizationName: 'filasieno',
  
  themeConfig: {
    navbar: {
      title: 'XInnoDB',
      items: [
        {to: '/docs/intro', label: 'Docs', position: 'left'},
        {to: '/blog', label: 'Blog', position: 'left'},
        {
          href: 'https://github.com/filasieno/innodb',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [...],
    },
  },
  
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
        },
        blog: {
          showReadingTime: true,
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
};
```

### Deploy

```bash
npm run build
npm run deploy
```

**Result**: Documentation at `https://filasieno.github.io/innodb`

---

## Option 3: Hugo (Very Fast)

### Setup

```bash
# Install Hugo
# (Use Nix: nix-shell -p hugo)

# Create site
hugo new site docs-site
cd docs-site

# Add theme
git submodule add https://github.com/theNewDynamic/gohugo-theme-ananke.git themes/ananke

# Directory structure
content/
├── _index.md
├── architecture.md
├── building.md
└── ...

config.toml               # Configuration
```

### Configuration (config.toml)

```toml
baseURL = "https://filasieno.github.io/innodb/"
languageCode = "en-us"
title = "XInnoDB Documentation"
theme = "ananke"

[params]
  description = "High-performance ABI-stable storage engine"
  github = "https://github.com/filasieno/innodb"

[menu]
  [[menu.main]]
    name = "Architecture"
    url = "/architecture/"
    weight = 1
  [[menu.main]]
    name = "Building"
    url = "/building/"
    weight = 2
```

### Deploy

```bash
hugo
# Output in public/

# Deploy to GitHub Pages
# Push public/ to gh-pages branch
```

---

## Option 4: VitePress (Modern Vue-based)

### Setup

```bash
npm init vitepress

# Directory structure
docs/
├── .vitepress/
│   └── config.js
├── index.md
├── architecture.md
└── ...
```

### Configuration

```javascript
// .vitepress/config.js
export default {
  title: 'XInnoDB',
  description: 'High-performance storage engine',
  
  themeConfig: {
    nav: [
      { text: 'Home', link: '/' },
      { text: 'Guide', link: '/getting-started' },
      { text: 'GitHub', link: 'https://github.com/filasieno/innodb' }
    ],
    
    sidebar: [
      {
        text: 'Introduction',
        items: [
          { text: 'Getting Started', link: '/getting-started' },
          { text: 'Architecture', link: '/architecture' }
        ]
      },
      {
        text: 'Development',
        items: [
          { text: 'Building', link: '/building' },
          { text: 'Contributing', link: '/contributing' }
        ]
      }
    ]
  }
}
```

---

## Comparison

| Tool | Language | Speed | Features | Learning Curve |
|------|----------|-------|----------|----------------|
| **MkDocs** | Python | Medium | Excellent for docs | Easy |
| **Docusaurus** | JavaScript | Fast | Very feature-rich | Medium |
| **Hugo** | Go | Very Fast | Flexible | Medium |
| **VitePress** | JavaScript | Very Fast | Modern, Vue | Easy-Medium |
| **Jekyll** | Ruby | Slow | Native GitHub Pages | Easy |

## Recommended Setup for XInnoDB

**Best Choice: MkDocs Material**

Why?
- ✅ Purpose-built for technical documentation
- ✅ Beautiful Material Design theme
- ✅ Great search functionality
- ✅ Code highlighting with syntax support
- ✅ Mermaid diagram support
- ✅ Easy to maintain (Python, single config file)
- ✅ Excellent mobile support

---

## Hybrid Approach: Wiki + Static Site

You can maintain both:

1. **GitHub Wiki** (Markdown) - Quick, simple documentation
2. **GitHub Pages** (Static site) - Comprehensive, searchable docs

### Workflow

```
wiki/                    # Simple markdown wiki
  └── (Basic docs)

docs/                    # Static site source
  └── (Comprehensive docs)

.github/workflows/
  └── docs.yml          # Auto-deploy to GitHub Pages
```

Convert wiki to static site:

```bash
# Copy wiki markdown to docs
cp wiki/*.md docs/

# Generate static site
mkdocs build

# Deploy
mkdocs gh-deploy
```

---

## Next Steps

1. **Choose a generator** (I recommend MkDocs Material)
2. **Set up the structure**
3. **Copy your existing wiki content**
4. **Configure GitHub Pages**
5. **Set up auto-deployment with GitHub Actions**

Would you like me to set up one of these for you?


