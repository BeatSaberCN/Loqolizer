# Loqolizer

Translate your game and mods. This mod is for player. For other part, refert to [this](https://github.com/frto027/ssl10n.csv/tree/master).

For this mod's developer...

Don't forget clone submoudles if you git clone this repo.

```
git clone --recursive xxx
```

or

```
git submodule init
git submodule update
```

Use `qpm s build` to build.  
Same goes for `qpm s copy` and `qpm s qmod`

# Embbed Data

This mod download translate from [ssl10n.csv](https://github.com/frto027/ssl10n.csv/tree/master) at build time, including the translate's version number.

**Only** the first version translate of every mods are included.

If you need old data, download an old mod. Or waiting for developing of this mod's Online Data feature.

# Version check

During load time, the Loqolizer find avaliable mods and load avaliable translates for them. Every translate can declare a mod version range. Therefore, modders don't need to worry about translate compatibility issues between mod versions.

We use [Neargye/semver](https://github.com/Neargye/semver) to match the mod version range. Specially, version number `*` for any version. Otherwise, follow the Readme file in that library.

## Embbed Data Version Check

When game start, the mod check every mod's id and version. The mismatched translate will be skipped.

## Online Data Version Check

WIP, online data download feature not avaliable.

The mod will iterates every avaliable version data for every mod, and use the first matched data.

## Auto release action

The action `auto-release.yml` will automatically make a draft release if `ssl10n.csv` is changed since the latest release. We use semver and it bumps the PATCH version number.

## Credits

* [zoller27osu](https://github.com/zoller27osu), [Sc2ad](https://github.com/Sc2ad) and [jakibaki](https://github.com/jakibaki) - [beatsaber-hook](https://github.com/sc2ad/beatsaber-hook)
* [raftario](https://github.com/raftario)
* [Lauriethefish](https://github.com/Lauriethefish), [danrouse](https://github.com/danrouse) and [Bobby Shmurner](https://github.com/BobbyShmurner) for [this template](https://github.com/Lauriethefish/quest-mod-template)
