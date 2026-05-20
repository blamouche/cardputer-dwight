# Tâche : créer le repo GitHub + tags/releases avec .bin

Timestamp : 20260520-033349
Slug : github-repo-releases

## Objectif
Publier le firmware Cardputer sur un nouveau repo GitHub (`cardputer-dwight`,
public, compte blamouche). Mettre en place la création de tags et de releases.
Chaque release doit embarquer le fichier `firmware.bin` mono-fichier (flash 0x0)
à installer sur le Cardputer.

## Décisions (validées user)
- Repo : `cardputer-dwight`, **public**.
- Source du .bin : **build en CI** (GitHub Actions) sur chaque tag → attaché à
  la release. Source de vérité reproductible.

## Plan
1. [ ] Workflow CI `.github/workflows/release.yml` :
   - build PlatformIO sur push (sanity) + sur tag `v*`
   - sur tag : compile, récupère `firmware.bin`, crée la release et y attache le .bin
2. [ ] `.gitignore` : exclure le binaire de build (le .bin vient de la CI).
3. [ ] `git init`, commit initial.
4. [ ] Bump version `.prompt-hub/version.md` 0.1.16 → 0.1.17 + releases.md.
5. [ ] Créer le repo GitHub public + push.
6. [ ] Créer le tag `v0.1.17` + release → vérifier que la CI attache le .bin.

## Review
(à compléter)
