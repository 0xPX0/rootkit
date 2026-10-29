# Rootkit LKM — Noyau Linux | École 2600

## Prérequis

- Linux kernel 6.x (testé sur Alpine 6.18.58-0-lts)
- Headers kernel installés : `apk add linux-lts-dev build-base`
- Accès root


## Compilation

```bash
make
```

Génère `rkit.ko` (module kernel) et `companion/rkit` (outil userland).

---

## Chargement

```bash
insmod rkit.ko
```

La persistance est automatique : au premier `insmod`, le module écrit son script de démarrage dans `/etc/local.d/rkit.start`. Il se rechargera automatiquement à chaque reboot.

---

## Comptes utilisateurs (VM de test)

| Utilisateur | Mot de passe |
|---|---|
| `root` | `root2600` |
| `user` | `user42` |

---

## Outil companion

Copier le binaire dans un emplacement accessible :

```bash
cp companion/rkit /tmp/rkit
chmod +x /tmp/rkit
```

### Commandes disponibles

| Commande | Effet |
|---|---|
| `/tmp/rkit root` | Élévation de privilèges → shell root |
| `/tmp/rkit hide <PID>` | Cache un processus par son PID |
| `/tmp/rkit show` | Révèle le module (pour rmmod) |

---

## Fonctionnalités

- **Cache de fichiers** : tout fichier/dossier préfixé `rkit_` est invisible dans `ls`
- **Cache de processus** : un PID ajouté via `hide` disparaît de `ps`
- **Auto-dissimulation** : le module ne s'affiche pas dans `lsmod`
- **Persistance** : rechargement automatique après reboot
- **Élévation de privilèges** : obtention d'un shell root depuis un compte non-privilégié
- **Hook read** : masque la ligne de persistance dans `/etc/local.d/rkit.start` à la lecture

---

## Déchargement

```bash
/tmp/rkit show   # révèle le module
rmmod rkit
```
