# 🎮 Progetto Unreal Engine 5.8 – Repository Iniziale

Benvenuto nel repository principale del progetto di gioco sviluppato con **Unreal Engine 5.8**.  
Questo repository raccoglie **codice, asset e configurazioni** necessari allo sviluppo condiviso del progetto.

---

## 🧩 Scopo del Repository

L’obiettivo di questo repository è centralizzare tutti i componenti essenziali del progetto:
- Codice sorgente C++ e Blueprint.
- Asset grafici, sonori e di gameplay.
- Configurazioni di progetto e impostazioni di build.

Per l’ambiente di sviluppo locale, consulta la guida dedicata:  
📄 **[DEVELOPER_SETUP.md](DEVELOPER_SETUP.md)**

---

## 📁 Struttura Principale

| Cartella | Contenuto |
|-----------|------------|
| `Source/` | Codice C++ del progetto (moduli, classi, gameplay logic). |
| `Content/` | Asset Unreal (`.uasset`, `.umap`, materiali, texture, suoni, ecc.). |
| `Config/` | File di configurazione (`Default*.ini`, impostazioni di build e progetto). |

---

## ⚙️ Policy e Regole Rapide

- 💾 **Usa Git LFS** per tutti i file binari (`*.uasset`, `*.umap`, `*.fbx`, ecc.).
- 🔀 **Crea una Pull Request (PR)** per ogni modifica non banale.
- 👥 **Richiedi almeno una review** prima di effettuare il merge.
- 📋 **Segui le linee guida** definite in:
  - [`CONTRIBUTING.md`](CONTRIBUTING.md)
  - [`DEVELOPER_SETUP.md`](DEVELOPER_SETUP.md)

---

## ⚠️ Nota Importante – Limiti del Piano GitHub Gratuito

Il piano GitHub gratuito ha **quote limitate** per:
- **Git LFS (storage e transfer)**
- **GitHub Actions (build minutes)**

Per evitare problemi di spazio o blocchi nei push:
1. **Non caricare asset Unreal di grandi dimensioni** (`.uasset`, `.umap`, ecc.) nel repository principale se la quota LFS non è sufficiente.
2. **Alternative consigliate:**
   - 📦 Conserva gli asset pesanti in uno **storage esterno** (S3, Google Drive, NAS, ecc.) e versiona solo i riferimenti o i metadata.
   - 🗂️ Crea un **repository separato per gli asset** (privato) e gestisci gli accessi separatamente.
   - 💼 Valuta l’upgrade a un **piano GitHub superiore** o l’uso di un **VCS alternativo** (es. *Perforce*, *Plastic SCM*) per gli asset voluminosi.

Consulta sempre la sezione dedicata in **DEVELOPER_SETUP.md** per la procedura consigliata di sincronizzazione e configurazione.

---

## 🤝 Collaborazione

Il progetto è gestito in team. Ogni membro è invitato a:
- Mantenere una **storia Git pulita e leggibile**.
- Utilizzare naming coerenti per branch, asset e blueprint.
- Coordinarsi tramite **Issues** e **Pull Requests** per evitare conflitti.

Per le convenzioni di naming, workflow Git e standard di revisione, consulta il file  
📘 [`CONTRIBUTING.md`](CONTRIBUTING.md)

---

## 🧠 Risorse Utili

- [Documentazione ufficiale Unreal Engine 5.8](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-documentation)
- [Guida Git LFS](https://git-lfs.github.com/)
- [Documentazione GitHub su Branch Protection e Workflow](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/)

---

### ✨ Autori & Collaboratori
Team di sviluppo – Unreal Engine 5.8 Project  
(Repository configurato per uso collaborativo su piano GitHub gratuito)

---

