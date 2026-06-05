# Loqolizer

![GitHub Release](https://img.shields.io/github/v/release/BeatSaberCN/Loqolizer?label=release%20tag) ![GitHub Release](https://img.shields.io/github/v/release/BeatSaberCN/Loqolizer?include_prereleases&label=pre-release%20tag)




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

This mod download translate from [ssl10n.csv](https://github.com/frto027/ssl10n.csv) at build time, including the translate's version number.

# Version check

During load time, the Loqolizer find avaliable mods and load avaliable translates for them. Every translate can declare a mod version range. Therefore, modders don't need to worry about translate compatibility issues between mod versions.

We use [Neargye/semver](https://github.com/Neargye/semver) to match the mod version range. Specially, version number `*` for any version. Otherwise, follow the Readme file in that library.

## Embbed Data Version Check

When game start, the mod check every mod's id and version. The mismatched translate will be skipped.

## Online Data Version Check

WIP, online data download feature not avaliable.

The mod will iterates every avaliable version data for every mod, and use the first matched data.

# How to release this mod

For maintainer of this repo.

## Option 1: auto pre-release

In action `auto-release.yml`, the mod check if the latest release(including pre-release) is synchronized with `ssl10n.csv`. If not, auto bump version and make a pre-release. Based on the `master` branch.

The maintainer in this repo can test and update the pre-release to a latest-release.

## Option 2: manual release

Just push a version tag `v*.*.*`, this will trigger the action `build-ndk.yml` and generate a draft release.

`git tag v0.6.4 && git push origin v0.6.4`

Make sure your release will not break `auto pre-release`. The next time when auto release happens, the version number is based on your release.


## Credits

* [zoller27osu](https://github.com/zoller27osu), [Sc2ad](https://github.com/Sc2ad) and [jakibaki](https://github.com/jakibaki) - [beatsaber-hook](https://github.com/sc2ad/beatsaber-hook)
* [raftario](https://github.com/raftario)
* [Lauriethefish](https://github.com/Lauriethefish), [danrouse](https://github.com/danrouse) and [Bobby Shmurner](https://github.com/BobbyShmurner) for [this template](https://github.com/Lauriethefish/quest-mod-template)
