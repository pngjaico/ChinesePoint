(() => {
  const select = (selector, root = document) => root.querySelector(selector);
  const selectAll = (selector, root = document) => [...root.querySelectorAll(selector)];

  const menuToggle = select('.menu-toggle');
  const nav = select('.site-nav');
  if (menuToggle && nav) {
    const closeMenu = () => {
      menuToggle.setAttribute('aria-expanded', 'false');
      nav.classList.remove('is-open');
    };
    menuToggle.addEventListener('click', () => {
      const open = menuToggle.getAttribute('aria-expanded') !== 'true';
      menuToggle.setAttribute('aria-expanded', String(open));
      nav.classList.toggle('is-open', open);
    });
    nav.addEventListener('click', (event) => {
      if (event.target.closest('a')) closeMenu();
    });
    window.addEventListener('resize', () => {
      if (window.innerWidth > 760) closeMenu();
    });
  }

  selectAll('[data-year]').forEach((node) => { node.textContent = String(new Date().getFullYear()); });

  const revealItems = selectAll('.reveal');
  if (!('IntersectionObserver' in window) || window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
    revealItems.forEach((node) => node.classList.add('is-visible'));
  } else {
    const observer = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting) {
          entry.target.classList.add('is-visible');
          observer.unobserve(entry.target);
        }
      });
    }, { rootMargin: '0px 0px -7% 0px', threshold: 0.06 });
    revealItems.forEach((node) => observer.observe(node));
  }

  const setText = (selector, value) => {
    const node = select(selector);
    if (node && value !== undefined && value !== null) node.textContent = String(value);
  };

  const formatBytes = (bytes) => new Intl.NumberFormat('pt-BR').format(bytes) + ' bytes';

  async function loadReleaseManifest() {
    try {
      const response = await fetch('release-manifest.json', { cache: 'no-store' });
      if (!response.ok) throw new Error(`manifest HTTP ${response.status}`);
      const manifest = await response.json();
      const artifact = manifest?.artifact;
      if (manifest?.schema !== 1 || manifest?.target !== 'xteink-x4-pro' || !artifact) {
        throw new Error('manifest is not an X4 Pro release record');
      }

      setText('[data-status-title]', manifest.version.replace('ChinesePoint-', 'ChinesePoint '));
      setText('[data-status-badge]', artifact.installable ? 'Instalável' : 'Não instalável');
      setText('[data-status-hash]', `${artifact.sha256.slice(0, 8)}…${artifact.sha256.slice(-5)}`);
      setText('[data-release-version]', manifest.version.replace('ChinesePoint-', 'ChinesePoint '));
      setText('[data-release-badge]', artifact.installable ? 'Instalável' : 'Não instalável');
      setText('[data-release-target]', 'Xteink X4 Pro');
      setText('[data-release-environment]', artifact.environment);
      setText('[data-release-bytes]', formatBytes(artifact.bytes));
      setText('[data-release-sha]', artifact.sha256);
      setText('[data-release-commit]', manifest.source_commit.slice(0, 7));
      setText('[data-release-notes]', artifact.installable
        ? 'Release marcado como instalável no manifesto assinado.'
        : `O artefato continua bloqueado: ${artifact.installable_reason}`);
    } catch (error) {
      // The rendered fallback is intentionally conservative: it never enables a download.
      console.warn('ChinesePoint release manifest could not be loaded; using the blocked fallback.', error);
    }
  }

  loadReleaseManifest();
})();
