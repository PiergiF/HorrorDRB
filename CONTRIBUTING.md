# 🤝 Linee Guida per i Contributori

Grazie per contribuire a questo progetto!  
Per mantenere il repository **stabile, collaborativo e facilmente scalabile**, segui con attenzione le seguenti linee guida.

---

## 🧱 Struttura del Lavoro

1. **Non lavorare mai direttamente su `main`.**
   - Crea un branch dedicato per ogni attività:
     - `feature/<nome-feature>` → nuove funzionalità
     - `hotfix/<nome-fix>` → correzioni rapide
2. **Apri una Pull Request (PR)** al termine del lavoro.
   - Collega sempre un’Issue tramite `Fixes #<numero>` quando applicabile.
   - Richiedi **almeno una review** per modifiche a codice o livelli di gioco.
3. Mantieni i commit **piccoli e significativi**, con messaggi chiari:

---

## 💾 Gestione degli Asset (Git LFS)

- Usa **Git LFS** per tutti i file binari di Unreal:
 - Usa **Git LFS** per tutti i file binari di Unreal dove necessario.
- ⚠️ **Attenzione per il piano GitHub gratuito:**
- Non caricare asset Unreal di grandi dimensioni direttamente nel repository principale,
- a meno che non siano esplicitamente autorizzati.
- Per questo repository, la sola cartella autorizzata è `Content/Procedural_Labirint/`.
- Tutti gli altri asset devono essere gestiti tramite storage esterno o repository dedicato.
- Se necessario, usa:
  - **Storage esterno** (S3, Google Drive, NAS)  
  - **Repository dedicato** solo per gli asset
Consulta **DEVELOPER_SETUP.md** per i passaggi raccomandati.

---

## 🧹 Pulizia e File da Escludere

Non committare mai file o cartelle generate automaticamente da Unreal:
Questi file vengono rigenerati localmente e non devono far parte del versionamento.

---

## 👥 Review & Code Quality

- Ogni PR deve essere **revisionata da almeno un altro membro del team**.
- Evita merge diretti non revisionati.
- Controlla sempre che:
  - Il progetto compili correttamente prima del push.
  - Non siano stati accidentalmente committati file non necessari o troppo grandi.
  - Le modifiche rispettino lo stile e le convenzioni del team.

---

## ⚙️ Buone Pratiche di Collaborazione

- **Sincronizza frequentemente** con `main` per evitare conflitti.
- **Comunica le modifiche sugli asset condivisi** (mappe, blueprint comuni).
- **Testa in locale** prima di aprire una PR.
- Se un asset è già in uso da un altro membro, coordina l’attività tramite **Issues** o messaggi nel canale di comunicazione del team.

---

## 🧩 Risorse e Riferimenti

- 📘 [DEVELOPER_SETUP.md](DEVELOPER_SETUP.md) – Guida all’ambiente di sviluppo
- 🧰 [Git LFS Docs](https://git-lfs.github.com/)
- 🔧 [Guida alle Pull Request GitHub](https://docs.github.com/en/pull-requests)
- 🎮 [Unreal Engine Documentation](https://docs.unrealengine.com/5.6/en-US/)

---

Grazie per il tuo contributo!  
Insieme possiamo mantenere un progetto **pulito, scalabile e professionale** 🎯
