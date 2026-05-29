# Loqolizer

Translate your game and mods

Don't forget clone submoudles if you git clone this repop.

```
git clone --recursive xxx
```

or

```
git submodule init
git submodule update
```

Use `qpm s build` to build
Same goes for `qpm s copy` and `qpm s qmod`

This mod download translate from [ssl10n.csv](https://github.com/frto027/ssl10n.csv/tree/master) at build time.

# Version check

During load time, the Loqolizer find avaliable mods and load avaliable translates for them. Every translate can declare a mod version range. Therefore, modders don't need to worry about translate compatibility issues between mod versions.

We use [Neargye/semver](https://github.com/Neargye/semver) to match the mod version range. Specially, version number `*` for any version. Otherwise, follow the Readme file in that library.

## Embbed Data Version Check

At the mod build time, the mod **only** embbed the every mod's latest version translate, including the version number.

At the runtime, the mod check every mod's id and version. The mismatched translate will be skipped.

## Online Data Version Check

WIP, online data download feature not avaliable.

The mod will iterates every avaliable version data for every mod, and use the first matched data.

## Credits

* [zoller27osu](https://github.com/zoller27osu), [Sc2ad](https://github.com/Sc2ad) and [jakibaki](https://github.com/jakibaki) - [beatsaber-hook](https://github.com/sc2ad/beatsaber-hook)
* [raftario](https://github.com/raftario)
* [Lauriethefish](https://github.com/Lauriethefish), [danrouse](https://github.com/danrouse) and [Bobby Shmurner](https://github.com/BobbyShmurner) for [this template](https://github.com/Lauriethefish/quest-mod-template)
