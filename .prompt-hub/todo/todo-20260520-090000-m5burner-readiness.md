# Todo — Vérification readiness M5Burner

## Objectif
Vérifier que le firmware Dwight Assistant est prêt à être publié sur M5Burner
(USER CUSTOM > Publish).

## Vérifications
- [x] Binaire compilé flashable @0x0 : `firmware.bin` = image fusionnée
      (bootloader@0x0 + partitions@0x8000 + boot_app0@0xe000 + app@0x10000),
      magic 0xE9, 1.23 MB. Format attendu par M5Burner. OK.
- [x] Version embarquée alignée : rebuild → Version.h = 0.1.19 (était 0.1.17 sur
      le .bin disque, antérieur aux derniers bumps). OK.
- [x] Device Type : Cardputer ADV (ESP32-S3, board m5stack-stamps3, flash 8MB).
      → choisir "Cardputer" dans M5Burner.
- [x] GitHub public : https://github.com/blamouche/cardputer-dwight. OK.
- [x] Description : README complet (apps, schéma config, build/flash). OK.
- [ ] Cover : ABSENTE. M5Burner demande une image de couverture → à créer
      (composable depuis docs/screenshots/).
- [ ] (Optionnel) Release GitHub à jour : la dernière est v0.1.17 ; tag v0.1.19
      conseillé pour cohérence.

## Verdict
PRÊT côté binaire/format/doc. Manque uniquement la **cover** (recommandée, pas
strictement bloquante) et, optionnellement, un tag/release v0.1.19.

## Review
- Statut : completed (vérification). Actions de suivi proposées à l'utilisateur.
